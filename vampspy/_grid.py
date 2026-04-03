"""
_grid.py — Multi-column / grid runner using multiprocessing + shared memory.

Each worker process gets an exclusive copy of the C global state (because
processes don't share address space), so the single-column Richards solver
can be used in parallel without any thread-safety issues.

Data layout
-----------
Forcing is packed into one shared-memory block: shape (nvars, ncols, steps).
Workers read column slices (views) — no copying of forcing data.

Results are written into separate shared-memory blocks:
  scalars  : (ncols, steps, nscalars)   — 15 scalar outputs per step
  theta    : (ncols, steps, nlayers)
  k, h, qrot, howsat : (ncols, steps, nlayers)
  q, inq   : (ncols, steps, nlayers+1)
  gwl      : (ncols, steps, 2)

The parent copies results from shared memory to ordinary numpy arrays before
closing the shared-memory blocks.
"""

from __future__ import annotations

import numpy as np
from multiprocessing import Pool, shared_memory, cpu_count

# Must match soil_api.h / _vampscore.c
_SCALAR_KEYS = (
    "t", "volact", "SMD", "qtop", "qbot", "avgtheta",
    "cumprec", "cumtra", "cumeva", "cumintc", "masbal",
    "precipitation", "interception", "transpiration", "soilevaporation",
)
_PROFILE_KEYS = ("theta", "k", "h", "qrot", "howsat", "q", "inq", "gwl")

# ---------------------------------------------------------------------------
# Worker-process state — populated once per process by _worker_init
# ---------------------------------------------------------------------------

_W: dict = {}


def _worker_init(worker_cfg: dict) -> None:
    """Attach to shared-memory blocks and cache config in the worker process.

    Called once per worker at Pool start.  Stores numpy views into the
    shared-memory buffers so _worker_chunk can read/write without copying.
    """
    global _W
    _W = dict(worker_cfg)   # copies scalars: ini_text, firststep, forcing_keys

    def _attach(name_key: str, shape_key: str) -> tuple:
        shm = shared_memory.SharedMemory(name=worker_cfg[name_key])
        arr = np.ndarray(worker_cfg[shape_key], dtype=np.float64, buffer=shm.buf)
        return shm, arr

    shm, arr = _attach('forcing_shm', 'forcing_shape')
    _W['_shm_forcing'] = shm
    _W['forcing_arr'] = arr

    shm, arr = _attach('scalars_shm', 'scalars_shape')
    _W['_shm_scalars'] = shm
    _W['scalars_arr'] = arr

    for key in _PROFILE_KEYS:
        shm, arr = _attach(f'{key}_shm', f'{key}_shape')
        _W[f'_shm_{key}'] = shm
        _W[f'{key}_arr'] = arr


def _worker_chunk(col_range: tuple) -> None:
    """Process columns [col_start, col_end) sequentially.

    Reads forcing from shared memory (zero copy), calls soil_init + step loop
    for each column, writes scalar and profile results back into shared memory.
    """
    from vampspy import _vampscore  # imported here so spawn workers get it

    col_start, col_end = col_range
    forcing_keys: list  = _W['forcing_keys']
    ini_text: str       = _W['ini_text']
    firststep: float    = _W['firststep']
    forcing_arr         = _W['forcing_arr']   # (nvars, ncols, steps)
    scalars_arr         = _W['scalars_arr']   # (ncols, steps, nscalars)
    steps: int          = forcing_arr.shape[2]

    for col in range(col_start, col_end):
        # Column-specific forcing slices — views into shared memory, no copy
        forcing = {key: forcing_arr[i, col, :] for i, key in enumerate(forcing_keys)}

        _vampscore.soil_init(ini_text, forcing, firststep)

        for step in range(steps):
            _vampscore.soil_step(step)
            s = _vampscore.soil_state_current()

            for j, sk in enumerate(_SCALAR_KEYS):
                scalars_arr[col, step, j] = s[sk]

            for pk in _PROFILE_KEYS:
                v = s[pk]
                _W[f'{pk}_arr'][col, step, :len(v)] = v


# ---------------------------------------------------------------------------
# Public entry point
# ---------------------------------------------------------------------------

def run_grid(
    ini_text: str,
    forcing_grid: dict,
    forcing_keys: list,
    firststep: float = 1.0,
    nworkers: int | None = None,
    chunk_size: int | None = None,
) -> dict:
    """Run the VAMPS soil solver for *ncols* independent columns in parallel.

    Parameters
    ----------
    ini_text      : INI config string (same soil profile for all columns).
    forcing_grid  : dict[str, ndarray] with values of shape (ncols, steps).
    forcing_keys  : ordered key list — determines packing order in shared mem.
    firststep     : x-value of the first timestep (default 1.0).
    nworkers      : worker processes (default: all CPU cores).
    chunk_size    : columns per task; None → ncols // (nworkers * 4), min 1.

    Returns
    -------
    dict
      scalar keys  → ndarray (ncols, steps)
      profile keys → ndarray (ncols, steps, profile_len)
      '_steps', '_nlayers'
    """
    from vampspy import _vampscore

    first = forcing_grid[forcing_keys[0]]
    ncols, steps = first.shape[0], first.shape[1]
    nvars = len(forcing_keys)

    nworkers = nworkers or cpu_count()
    nworkers = min(nworkers, ncols)
    if chunk_size is None:
        chunk_size = max(1, ncols // (nworkers * 4))

    # ------------------------------------------------------------------
    # Probe run on column 0 → nlayers + per-profile sizes
    # ------------------------------------------------------------------
    probe_forcing = {key: forcing_grid[key][0, :] for key in forcing_keys}
    _vampscore.soil_init(ini_text, probe_forcing, firststep)
    nlayers = _vampscore.soil_nlayers()
    _vampscore.soil_step(0)
    probe = _vampscore.soil_state_current()
    profile_sizes = {key: len(probe[key]) for key in _PROFILE_KEYS}

    # ------------------------------------------------------------------
    # Allocate shared memory
    # ------------------------------------------------------------------
    shm_blocks: list = []

    def _make_shm(shape: tuple):
        nbytes = max(int(np.prod(shape)) * 8, 8)  # float64, at least 8 B
        shm = shared_memory.SharedMemory(create=True, size=nbytes)
        arr = np.ndarray(shape, dtype=np.float64, buffer=shm.buf)
        shm_blocks.append(shm)
        return shm, arr

    forcing_shape  = (nvars, ncols, steps)
    scalars_shape  = (ncols, steps, len(_SCALAR_KEYS))

    shm_forcing,  forcing_arr  = _make_shm(forcing_shape)
    shm_scalars,  scalars_arr  = _make_shm(scalars_shape)

    profile_shms: dict = {}
    profile_arrs: dict = {}
    for key in _PROFILE_KEYS:
        pshape = (ncols, steps, profile_sizes[key])
        shm, arr = _make_shm(pshape)
        profile_shms[key] = shm
        profile_arrs[key] = arr

    # Pack forcing into shared memory
    for i, key in enumerate(forcing_keys):
        forcing_arr[i, :, :] = forcing_grid[key]

    # ------------------------------------------------------------------
    # Build worker config dict (pickled once per worker at pool start)
    # ------------------------------------------------------------------
    worker_cfg: dict = {
        'ini_text':      ini_text,
        'firststep':     firststep,
        'forcing_keys':  forcing_keys,
        'forcing_shm':   shm_forcing.name,
        'forcing_shape': forcing_shape,
        'scalars_shm':   shm_scalars.name,
        'scalars_shape': scalars_shape,
    }
    for key in _PROFILE_KEYS:
        worker_cfg[f'{key}_shm']   = profile_shms[key].name
        worker_cfg[f'{key}_shape'] = profile_arrs[key].shape

    # ------------------------------------------------------------------
    # Build column-range tasks
    # ------------------------------------------------------------------
    chunks = [
        (start, min(start + chunk_size, ncols))
        for start in range(0, ncols, chunk_size)
    ]

    # ------------------------------------------------------------------
    # Run the pool
    # ------------------------------------------------------------------
    try:
        with Pool(
            processes=nworkers,
            initializer=_worker_init,
            initargs=(worker_cfg,),
        ) as pool:
            pool.map(_worker_chunk, chunks)

        # Copy results before releasing shared memory
        result: dict = {}
        for j, key in enumerate(_SCALAR_KEYS):
            result[key] = np.array(scalars_arr[:, :, j])
        for key in _PROFILE_KEYS:
            result[key] = np.array(profile_arrs[key])
        result['_steps']   = steps
        result['_nlayers'] = nlayers

    finally:
        for shm in shm_blocks:
            try:
                shm.close()
                shm.unlink()
            except Exception:
                pass

    return result
