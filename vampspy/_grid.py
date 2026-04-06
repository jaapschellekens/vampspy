"""
_grid.py — Multi-column / grid runner using multiprocessing + shared memory.

Each worker process gets an exclusive copy of the C global state (because
processes don't share address space), so the single-column Richards solver
can be used in parallel without any thread-safety issues.

Data layout
-----------
Forcing is packed into one shared-memory block: shape (nvars, ncols, steps).
Workers read column slices (views) — no copying of forcing data.

Results are written into separate shared-memory blocks sized to the number
of *saved* timesteps (n_save ≤ steps), not the full run length:
  scalars  : (ncols, n_save, nscalars)   — 15 scalar outputs per saved step
  theta    : (ncols, n_save, nlayers)
  k, h, qrot, howsat : (ncols, n_save, nlayers)
  q, inq   : (ncols, n_save, nlayers+1)
  gwl      : (ncols, n_save, 2)

The caller selects which timesteps to retain via the ``save_steps`` argument;
the last step is always saved.  This keeps memory proportional to n_save
rather than to the full run length, enabling arbitrarily long simulations.
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
# Save-step resolution
# ---------------------------------------------------------------------------

def _resolve_save_indices(steps: int, save_steps) -> np.ndarray:
    """Return a sorted array of 0-based step indices that will be saved.

    Parameters
    ----------
    steps      : total number of timesteps in the run.
    save_steps : one of:
        None           → save every step (default / backwards-compatible).
        int N          → save every N-th step (steps N-1, 2N-1, …).
        sequence       → save these specific 0-based step indices.

    The last step (``steps - 1``) is always included regardless of the
    choice above.
    """
    last = steps - 1
    if save_steps is None:
        return np.arange(steps, dtype=np.intp)
    if isinstance(save_steps, (int, np.integer)):
        n = int(save_steps)
        if n <= 0:
            raise ValueError("save_steps integer must be >= 1")
        idx = np.arange(n - 1, steps, n, dtype=np.intp)
    else:
        idx = np.array(sorted(set(int(i) for i in save_steps)), dtype=np.intp)

    # Ensure last step is always included
    if len(idx) == 0 or idx[-1] != last:
        idx = np.append(idx, last)
    return idx


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

    # Reconstruct O(1) lookup structure for save indices
    _W['save_set'] = set(_W['save_indices'])   # set for fast membership test

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
    for each column.  At each timestep, state is written to the output shared-
    memory blocks *only* if the step index is in the save set.  This keeps
    memory usage proportional to n_save rather than to the total run length.
    """
    from vampspy import _vampscore  # imported here so spawn workers get it

    col_start, col_end = col_range
    forcing_keys: list  = _W['forcing_keys']
    ini_text: str       = _W['ini_text']
    firststep: float    = _W['firststep']
    forcing_arr         = _W['forcing_arr']   # (nvars, ncols, steps)
    scalars_arr         = _W['scalars_arr']   # (ncols, n_save, nscalars)
    save_set: set       = _W['save_set']
    save_indices: list  = _W['save_indices']  # sorted list — used for slot lookup
    steps: int          = forcing_arr.shape[2]

    for col in range(col_start, col_end):
        # Column-specific forcing slices — views into shared memory, no copy
        forcing = {key: forcing_arr[i, col, :] for i, key in enumerate(forcing_keys)}

        _vampscore.soil_init(ini_text, forcing, firststep)

        slot = 0   # index into the n_save output dimension
        for step in range(steps):
            _vampscore.soil_step(step)

            if step in save_set:
                s = _vampscore.soil_state_current()

                for j, sk in enumerate(_SCALAR_KEYS):
                    scalars_arr[col, slot, j] = s[sk]

                for pk in _PROFILE_KEYS:
                    v = s[pk]
                    _W[f'{pk}_arr'][col, slot, :len(v)] = v

                slot += 1


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
    save_steps=None,
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
    save_steps    : which timesteps to retain in the output.
        None      → all steps (default; backwards-compatible).
        int N     → every N-th step (0-based: N-1, 2N-1, …) plus the last.
        sequence  → these specific 0-based step indices, plus the last.

    Returns
    -------
    dict
      scalar keys  → ndarray (ncols, n_save)
      profile keys → ndarray (ncols, n_save, profile_len)
      '_steps'     → total steps run
      '_nlayers'   → number of soil layers
      '_save_steps'→ 0-based indices of the saved timesteps (length n_save)
    """
    from vampspy import _vampscore

    first = forcing_grid[forcing_keys[0]]
    ncols, steps = first.shape[0], first.shape[1]
    nvars = len(forcing_keys)

    nworkers = nworkers or cpu_count()
    nworkers = min(nworkers, ncols)
    if chunk_size is None:
        chunk_size = max(1, ncols // (nworkers * 4))

    save_indices = _resolve_save_indices(steps, save_steps)
    n_save = len(save_indices)

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
    # Allocate shared memory — output sized to n_save, not steps
    # ------------------------------------------------------------------
    shm_blocks: list = []

    def _make_shm(shape: tuple):
        nbytes = max(int(np.prod(shape)) * 8, 8)  # float64, at least 8 B
        shm = shared_memory.SharedMemory(create=True, size=nbytes)
        arr = np.ndarray(shape, dtype=np.float64, buffer=shm.buf)
        shm_blocks.append(shm)
        return shm, arr

    forcing_shape  = (nvars, ncols, steps)
    scalars_shape  = (ncols, n_save, len(_SCALAR_KEYS))

    shm_forcing,  forcing_arr  = _make_shm(forcing_shape)
    shm_scalars,  scalars_arr  = _make_shm(scalars_shape)

    profile_shms: dict = {}
    profile_arrs: dict = {}
    for key in _PROFILE_KEYS:
        pshape = (ncols, n_save, profile_sizes[key])
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
        'save_indices':  save_indices.tolist(),   # plain list — pickle-friendly
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
        result['_steps']      = steps
        result['_nlayers']    = nlayers
        result['_save_steps'] = save_indices

    finally:
        for shm in shm_blocks:
            try:
                shm.close()
                shm.unlink()
            except Exception:
                pass

    return result
