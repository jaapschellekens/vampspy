"""
model.py — High-level Model class for running VAMPS simulations from Python.

Three execution paths are available:

* ``run()``            — whole-run API (C drives the loop); uses the compiled
                         _vampscore extension when available, otherwise falls
                         back to the subprocess/file approach.
* ``run_stepwise()``   — Python drives the external timestep loop; C handles
                         canopy + Richards solver per step.  Requires the
                         _vampscore extension.
* ``_run_subprocess()``— subprocess fallback (Phase 1).
"""

from __future__ import annotations
import copy
import os
import subprocess
import tempfile  # used by _run_subprocess only

import numpy as np

from vampspy._io import write_ts, write_inp, write_inp_str, read_out, parse_inp
from vampspy.forcing import load_ts_spec

# Try to import the compiled C extension (Phase 2)
try:
    from vampspy import _vampscore
    _HAS_CORE = True
except ImportError:
    _HAS_CORE = False

_DEFAULT_EXECUTABLE = os.environ.get("VAMPS_BINARY", "vamps")


class Model:
    """Run a VAMPS 1D soil-water simulation from Python.

    Parameters
    ----------
    config : dict
        Nested dict mirroring the INI sections of a .inp file.
        Do not include 'run.outputfile', 'time.firststep', or 'ts'
        paths — those are managed automatically.
    forcing : dict[str, array-like]
        Time-series forcing variables.  Keys match the variable names
        used in the [ts] section (e.g. 'pre', 'nra', 'tem', ...).
        Values are 1-D array-like objects of length ``steps``.
        The number of steps is inferred from the forcing arrays.
    executable : str, optional
        Path to the vamps binary (Phase 1 fallback only).
        Defaults to the VAMPS_BINARY environment variable, or 'vamps'.
    vampslib : str, optional
        Path to the VAMPS share directory (Phase 1 only).
        Defaults to the VAMPSLIB environment variable.
    """

    def __init__(
        self,
        config: dict,
        forcing: dict[str, "np.ndarray | list"],
        executable: str = _DEFAULT_EXECUTABLE,
        vampslib: str | None = None,
    ):
        self.config = config
        self.forcing = {k: np.asarray(v, dtype=float) for k, v in forcing.items()}
        self.executable = executable
        self.vampslib = vampslib or os.environ.get("VAMPSLIB", "")

        lengths = {k: len(v) for k, v in self.forcing.items()}
        if len(set(lengths.values())) > 1:
            raise ValueError(
                f"All forcing arrays must have the same length; got: {lengths}"
            )

    @classmethod
    def from_file(
        cls,
        inp_path: str,
        executable: str = _DEFAULT_EXECUTABLE,
        vampslib: str | None = None,
    ) -> "Model":
        """Create a Model by reading an existing VAMPS .inp config file.

        Python parses the file and loads all forcing time series referenced in
        the ``[ts]`` section.  Special ts keywords that C handles internally
        (e.g. ``hea = hg``) are kept in the config dict so C can still act on
        them; everything backed by a real file is loaded into numpy arrays.

        The ``[run]`` and ``[xout]`` sections are stripped — the Model manages
        output paths itself.  Logging and interactive settings in ``[vamps]``
        are silenced so the run is self-contained.

        Parameters
        ----------
        inp_path  : path to the .inp file.
        executable: path to the vamps binary (Phase 1 fallback only).
        vampslib  : path to VAMPS share dir (Phase 1 fallback only).
        """
        import os
        inp_path = os.path.abspath(inp_path)
        base_dir = os.path.dirname(inp_path)

        sections = parse_inp(inp_path)

        # Load forcing from [ts] section; keep non-file entries for C
        ts_section = sections.pop("ts", {})
        forcing: dict[str, np.ndarray] = {}
        passthrough_ts: dict = {}

        for name, spec in ts_section.items():
            arr = load_ts_spec(str(spec), base_dir)
            if arr is not None:
                forcing[name] = arr
            else:
                passthrough_ts[name] = spec  # e.g. "hea = hg"

        if passthrough_ts:
            sections["ts"] = passthrough_ts

        # Clip all forcing arrays to the step count declared in [time] steps,
        # if present.  Forcing files often contain more data than the run needs
        # (e.g. vamps_demo has 91-92 rows but steps = 61).
        declared_steps = int(sections.get("time", {}).get("steps", 0))
        if declared_steps > 0 and forcing:
            forcing = {k: v[:declared_steps] for k, v in forcing.items()}

        # Strip output-related sections (Model manages these)
        for key in ("run", "xout"):
            sections.pop(key, None)

        # Silence logging and interactive features so the run is self-contained
        vamps_sec = sections.setdefault("vamps", {})
        vamps_sec["logging"] = 0
        vamps_sec.pop("xtrasl", None)
        vamps_sec.pop("xtrapy", None)

        return cls(
            config=sections,
            forcing=forcing,
            executable=executable,
            vampslib=vampslib,
        )

    @property
    def steps(self) -> int:
        if self.forcing:
            return len(next(iter(self.forcing.values())))
        return int(self.config.get("time", {}).get("steps", 0))

    def run(self, firststep: float = 1.0) -> dict:
        """Run the simulation and return results as numpy arrays.

        Uses the compiled C extension (_vampscore) when available —
        no subprocess, no temp files for forcing or output.
        Falls back to the subprocess/file method otherwise.

        Parameters
        ----------
        firststep : float
            x-value of the first forcing timestep (default 1.0).

        Returns
        -------
        dict
            Scalar time-series → ndarray of shape (steps,).
            Profile time-series (theta) → ndarray of shape (steps, nlayers).
            Special keys: '_steps', '_nlayers'.
        """
        if _HAS_CORE:
            return self._run_core(firststep)
        return self._run_subprocess(firststep)

    # ------------------------------------------------------------------
    # Phase 2: in-process via C extension
    # ------------------------------------------------------------------
    def _run_core(self, firststep: float = 1.0) -> dict:
        cfg = copy.deepcopy(self.config)
        cfg.setdefault("time", {})["steps"] = self.steps
        cfg["ts"] = {name: f"_array_{name}" for name in self.forcing}

        ini_text = write_inp_str(cfg, "/dev/null", {}, firststep=int(firststep))
        return _vampscore.run(ini_text, self.forcing, firststep)

    # ------------------------------------------------------------------
    # Grid / batch: parallel columns via multiprocessing + shared memory
    # ------------------------------------------------------------------
    def run_grid(
        self,
        forcing_grid: "dict[str, np.ndarray]",
        nworkers: "int | None" = None,
        chunk_size: "int | None" = None,
        firststep: float = 1.0,
        save_steps=None,
    ) -> dict:
        """Run the simulation for multiple soil columns in parallel.

        Each column shares the same soil-profile config (``self.config``) but
        receives its own forcing time series.  The outer loop is distributed
        across worker processes using shared memory so no forcing data is
        copied over IPC.

        Parameters
        ----------
        forcing_grid : dict[str, array-like]
            Forcing arrays of shape ``(ncols, steps)`` for a flat batch, or
            ``(ny, nx, steps)`` for a 2-D spatial grid.  All spatial
            dimensions are collapsed internally; results are reshaped back.
        nworkers : int, optional
            Number of worker processes.  Defaults to ``os.cpu_count()``.
        chunk_size : int, optional
            Columns per task dispatched to each worker.  Smaller values give
            better load-balancing at the cost of more IPC; larger values
            reduce overhead.  Default: ``ncols // (nworkers * 4)``.
        firststep : float
            x-value of the first forcing timestep (default 1.0).
        save_steps : None | int | sequence of int, optional
            Which timesteps to retain in the output arrays.
            ``None``  — keep all steps (default; backwards-compatible).
            ``int N`` — keep every N-th step (0-based: N-1, 2N-1, …).
            sequence  — keep these specific 0-based step indices.
            The last step is always kept regardless of this setting.
            Output arrays have shape ``(*spatial, n_save)`` or
            ``(*spatial, n_save, profile_len)`` where ``n_save`` is the
            number of retained steps.  ``result['_save_steps']`` holds the
            corresponding 0-based step indices.

        Returns
        -------
        dict
            Scalar keys → ndarray of shape ``(*spatial, n_save)``.
            Profile keys (theta, h, …) → ndarray of shape
            ``(*spatial, n_save, profile_len)``.
            Special keys: ``'_steps'`` (total steps run), ``'_nlayers'``,
            ``'_save_steps'`` (0-based indices of retained timesteps).
        """
        if not _HAS_CORE:
            raise RuntimeError(
                "run_grid() requires the _vampscore C extension. "
                "Build it with:  pip install -e ."
            )
        from vampspy._grid import run_grid as _run_grid

        # Normalise to float64 and determine shapes
        fg = {k: np.asarray(v, dtype=float) for k, v in forcing_grid.items()}
        first      = next(iter(fg.values()))
        spatial    = first.shape[:-1]          # e.g. () for 1-D, (ny, nx) for grid
        steps      = first.shape[-1]
        ncols      = int(np.prod(spatial)) if spatial else 1

        # Flatten spatial dims → (ncols, steps)
        fg_flat = {k: v.reshape(ncols, steps) for k, v in fg.items()}
        forcing_keys = list(fg_flat.keys())

        # Build INI text (same config for all columns)
        cfg = copy.deepcopy(self.config)
        cfg.setdefault("time", {})["steps"] = steps
        cfg["ts"] = {name: f"_array_{name}" for name in fg_flat}
        ini_text = write_inp_str(cfg, "/dev/null", {}, firststep=int(firststep))

        result_flat = _run_grid(
            ini_text=ini_text,
            forcing_grid=fg_flat,
            forcing_keys=forcing_keys,
            firststep=firststep,
            nworkers=nworkers,
            chunk_size=chunk_size,
            save_steps=save_steps,
        )

        # Reshape results back to original spatial dimensions.
        # Output time axis is n_save (may differ from steps when save_steps is set).
        n_save = len(result_flat['_save_steps'])
        result: dict = {}
        for key, arr in result_flat.items():
            if key.startswith("_"):
                result[key] = arr
                continue
            if arr.ndim == 2:                      # (ncols, n_save)
                result[key] = arr.reshape(spatial + (n_save,))
            elif arr.ndim == 3:                    # (ncols, n_save, profile_len)
                result[key] = arr.reshape(spatial + arr.shape[1:])
            else:
                result[key] = arr
        return result

    # ------------------------------------------------------------------
    # Stepwise: Python drives the external loop, C runs each step
    # ------------------------------------------------------------------
    def run_stepwise(self, firststep: float = 1.0) -> dict:
        """Run the simulation with Python controlling the external timestep loop.

        For each external step Python calls ``_vampscore.soil_step(i)``, which
        internally runs the C canopy (``tstep_top``) and the Richards solver
        with its own adaptive sub-stepping.  Python then collects state via
        ``_vampscore.soil_state(i)`` and assembles the final result arrays.

        This is functionally identical to ``run()`` but the outer loop lives
        here in Python, making it straightforward to inspect or modify state
        between timesteps.

        Requires the compiled _vampscore C extension.
        """
        if not _HAS_CORE:
            raise RuntimeError(
                "run_stepwise() requires the _vampscore C extension. "
                "Build it with:  pip install -e ."
            )

        cfg = copy.deepcopy(self.config)
        cfg.setdefault("time", {})["steps"] = self.steps
        cfg["ts"] = {name: f"_array_{name}" for name in self.forcing}

        ini_text = write_inp_str(cfg, "/dev/null", {}, firststep=int(firststep))
        _vampscore.soil_init(ini_text, self.forcing, firststep)

        per_step = []
        for i in range(self.steps):
            _vampscore.soil_step(i)
            per_step.append(_vampscore.soil_state(i))

        return _assemble_stepwise(per_step)

    # ------------------------------------------------------------------
    # Phase 1 fallback: subprocess + temp files
    # ------------------------------------------------------------------
    def _run_subprocess(self, firststep: float = 1.0) -> dict:
        firststep_i = int(firststep)
        with tempfile.TemporaryDirectory(prefix="vamps_") as tmpdir:
            ts_paths: dict[str, str] = {}
            for name, arr in self.forcing.items():
                ts_file = os.path.join(tmpdir, f"{name}.ts")
                write_ts(arr, ts_file, firststep=firststep_i)
                ts_paths[name] = ts_file

            inp_path = os.path.join(tmpdir, "run.inp")
            out_path = os.path.join(tmpdir, "run.out")

            cfg = copy.deepcopy(self.config)
            cfg.setdefault("time", {})["steps"] = self.steps
            write_inp(cfg, inp_path, out_path, ts_paths, firststep=firststep_i)

            env = os.environ.copy()
            if self.vampslib:
                env["VAMPSLIB"] = self.vampslib

            result = subprocess.run(
                [self.executable, "-o", out_path, inp_path],
                capture_output=True, text=True, cwd=tmpdir, env=env,
            )

            if result.returncode != 0:
                raise RuntimeError(
                    f"vamps exited with code {result.returncode}.\n"
                    f"stdout:\n{result.stdout}\n"
                    f"stderr:\n{result.stderr}"
                )

            if not os.path.exists(out_path):
                raise RuntimeError(
                    f"vamps finished but output file not found: {out_path}\n"
                    f"stdout:\n{result.stdout}"
                )

            return read_out(out_path)


# ---------------------------------------------------------------------------
# Helper used by Model.run_stepwise()
# ---------------------------------------------------------------------------

_SCALAR_KEYS = (
    "t", "volact", "SMD", "qtop", "qbot", "avgtheta",
    "cumprec", "cumtra", "cumeva", "cumintc", "masbal",
    "precipitation", "interception", "transpiration", "soilevaporation",
)

# Per-layer profile keys returned by soil_state() as 1-D arrays.
# _assemble_stepwise stacks them into shape (steps, n) arrays.
_PROFILE_KEYS = ("theta", "k", "h", "q", "inq", "qrot", "howsat", "gwl")


def _assemble_stepwise(per_step: list[dict]) -> dict:
    """Convert a list of per-step state dicts into arrays.

    Mirrors the dict layout returned by ``_vampscore.run()``.
    """
    if not per_step:
        return {"_steps": 0, "_nlayers": 0}

    result: dict = {}
    for k in _SCALAR_KEYS:
        result[k] = np.array([s[k] for s in per_step], dtype=float)

    # Profile arrays: shape (steps, n) where n varies per variable
    for k in _PROFILE_KEYS:
        result[k] = np.array([s[k] for s in per_step], dtype=float)

    result["_steps"]   = len(per_step)
    result["_nlayers"] = int(per_step[0].get("_nlayers", result["theta"].shape[1]))

    return result
