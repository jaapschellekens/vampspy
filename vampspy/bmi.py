"""
vampspy.bmi — BMI 2.0 wrapper for the VAMPS 1-D unsaturated zone model.

A single ``VampsBmi`` instance represents one *or more* independent soil
columns that share the same profile configuration but receive independent
forcing.  Multi-column operation runs each column in a persistent worker
process (one process per column) so that the global C state in
``_vampscore`` is isolated per column.

**Configuration file** (YAML)::

    inp_file   : path/to/model.inp   # VAMPS .inp file (required)
    ncols      : 1                   # number of soil columns (default 1)
    firststep  : 1.0                 # x-value of first timestep (default 1.0)
    steps      : 61                  # total steps; default from [time].steps

**BMI variable names**

Input (forcing) — ``set_value`` before each ``update``:

=========================  ====  ============  ===========================
BMI name                   Grid  Units         VAMPS ts key
=========================  ====  ============  ===========================
precipitation              0     cm d-1        pre
net_radiation              0     W m-2         nra
incoming_radiation         0     W m-2         ira
relative_humidity          0     —  (0–1)      rlh
air_temperature            0     degC          tem
wind_speed                 0     m s-1         win
=========================  ====  ============  ===========================

Output scalars — ``get_value`` after each ``update``:

=========================  ====  ============  ===========================
BMI name                   Grid  Units         VAMPS key
=========================  ====  ============  ===========================
soil_water_volume          0     cm            volact
soil_moisture_deficit      0     cm            SMD
surface_runoff             0     cm timestep-1 qtop
bottom_drainage            0     cm timestep-1 qbot
avg_soil_water_content     0     —             avgtheta
precipitation_flux         0     cm timestep-1 precipitation
transpiration_flux         0     cm timestep-1 transpiration
soil_evaporation_flux      0     cm timestep-1 soilevaporation
interception_flux          0     cm timestep-1 interception
=========================  ====  ============  ===========================

Output profiles (one value per soil layer) — ``get_value`` after each ``update``:

=========================  ====  ============  ===========================
BMI name                   Grid  Units         VAMPS key
=========================  ====  ============  ===========================
soil_water_content         1     —             theta
pressure_head              1     cm            h
hydraulic_conductivity     1     cm d-1        k
=========================  ====  ============  ===========================

For ``ncols > 1`` all variable arrays have an extra leading dimension of
size ``ncols``.  Grid 0 has shape ``(ncols,)``; grid 1 has shape
``(ncols, nlayers)``, stored row-major (all layers of column 0 first).
"""
from __future__ import annotations

import copy
import multiprocessing as mp
import os
from typing import Any

import numpy as np
import yaml

import bmipy

from vampspy._io import write_inp_str, parse_inp
from vampspy.forcing import load_ts_spec

try:
    from vampspy import _vampscore
    _HAS_CORE = True
except ImportError:
    _HAS_CORE = False

# ---------------------------------------------------------------------------
# Variable catalogue
# ---------------------------------------------------------------------------

# VAMPS ts-section key → BMI input variable name
_TS_TO_BMI: dict[str, str] = {
    "pre": "precipitation",
    "nra": "net_radiation",
    "ira": "incoming_radiation",
    "rlh": "relative_humidity",
    "tem": "air_temperature",
    "win": "wind_speed",
}
_BMI_TO_TS: dict[str, str] = {v: k for k, v in _TS_TO_BMI.items()}

# BMI output variable name → soil_state_current() key
_SCALAR_OUTPUTS: dict[str, str] = {
    "soil_water_volume":      "volact",
    "soil_moisture_deficit":  "SMD",
    "surface_runoff":         "qtop",
    "bottom_drainage":        "qbot",
    "avg_soil_water_content": "avgtheta",
    "precipitation_flux":     "precipitation",
    "transpiration_flux":     "transpiration",
    "soil_evaporation_flux":  "soilevaporation",
    "interception_flux":      "interception",
}

_PROFILE_OUTPUTS: dict[str, str] = {
    "soil_water_content":     "theta",
    "pressure_head":          "h",
    "hydraulic_conductivity": "k",
}

_UNITS: dict[str, str] = {
    "precipitation":          "cm d-1",
    "net_radiation":          "W m-2",
    "incoming_radiation":     "W m-2",
    "relative_humidity":      "1",
    "air_temperature":        "degC",
    "wind_speed":             "m s-1",
    "soil_water_volume":      "cm",
    "soil_moisture_deficit":  "cm",
    "surface_runoff":         "cm",
    "bottom_drainage":        "cm",
    "avg_soil_water_content": "1",
    "precipitation_flux":     "cm",
    "transpiration_flux":     "cm",
    "soil_evaporation_flux":  "cm",
    "interception_flux":      "cm",
    "soil_water_content":     "1",
    "pressure_head":          "cm",
    "hydraulic_conductivity": "cm d-1",
}

# Grid identifiers
_GRID_SCALAR  = 0   # surface/scalar variables — shape (ncols,)
_GRID_PROFILE = 1   # soil profile variables   — shape (ncols, nlayers)


# ---------------------------------------------------------------------------
# Persistent worker for one soil column (used when ncols > 1)
# ---------------------------------------------------------------------------

def _column_worker(
    col_idx: int,
    ini_text: str,
    total_steps: int,
    firststep: float,
    forcing_names: list[str],
    cmd_q: "mp.Queue[Any]",
    result_q: "mp.Queue[Any]",
) -> None:
    """Worker process: owns one column's C state for its lifetime."""
    from vampspy import _vampscore  # imported fresh in each worker process
    import numpy as np

    # Pre-allocate forcing arrays; ts_register_array stores a pointer to
    # these buffers, so updating them before soil_step(i) is seen by C.
    forcing: dict[str, np.ndarray] = {
        name: np.zeros(total_steps, dtype=np.float64)
        for name in forcing_names
    }

    _vampscore.soil_init(ini_text, forcing, 1.0)  # always 1-based internally
    nlayers = _vampscore.soil_nlayers()
    result_q.put(("ready", col_idx, nlayers))

    while True:
        msg = cmd_q.get()
        if msg[0] == "step":
            _, step_idx, forcing_update = msg
            for name, value in forcing_update.items():
                _vampscore.soil_patch_ts(name, step_idx, float(value))
            _vampscore.soil_step(step_idx)
            state = _vampscore.soil_state(step_idx)
            result_q.put(("state", col_idx, state))
        elif msg[0] == "done":
            break


# ---------------------------------------------------------------------------
# VampsBmi
# ---------------------------------------------------------------------------

class VampsBmi(bmipy.Bmi):
    """BMI 2.0 interface for VAMPS.

    Supports single-column (``ncols=1``) and multi-column (``ncols>1``)
    operation.  In multi-column mode each column runs in its own worker
    process so that the global C state in ``_vampscore`` stays isolated.
    """

    def __init__(self) -> None:
        self._initialized = False

    # ------------------------------------------------------------------
    # Lifecycle
    # ------------------------------------------------------------------

    def initialize(self, config_file: str) -> None:
        """Initialise from a YAML configuration file.

        Parameters
        ----------
        config_file : str
            Path to the YAML config file.  See module docstring for format.
        """
        if not _HAS_CORE:
            raise RuntimeError(
                "VampsBmi requires the _vampscore C extension. "
                "Build it with:  pip install -e ."
            )

        config_file = os.path.abspath(config_file)
        base_dir = os.path.dirname(config_file)

        with open(config_file) as fh:
            cfg = yaml.safe_load(fh)

        inp_file = os.path.join(base_dir, cfg["inp_file"])
        self._ncols = int(cfg.get("ncols", 1))
        self._firststep = float(cfg.get("firststep", 1.0))

        # Parse .inp to discover ts variable names and soil config
        sections = parse_inp(inp_file)
        ts_section = sections.pop("ts", {})

        # Determine which ts variables have real file backing
        inp_dir = os.path.dirname(inp_file)
        self._forcing_names: list[str] = []
        for name, spec in ts_section.items():
            arr = load_ts_spec(str(spec), inp_dir)
            if arr is not None:
                self._forcing_names.append(name)

        # Derive steps: YAML overrides .inp [time].steps
        declared = int(sections.get("time", {}).get("steps", 0))
        self._steps = int(cfg.get("steps", declared))
        if self._steps <= 0:
            raise ValueError(
                "steps must be > 0; set it in the YAML config or "
                "[time] steps in the .inp file"
            )

        # Build INI text: strip ts file paths, register as array placeholders
        sections_ini = copy.deepcopy(sections)
        sections_ini.setdefault("time", {})["steps"] = self._steps
        sections_ini["ts"] = {
            name: f"_array_{name}" for name in self._forcing_names
        }
        # Silence interactive/logging output
        vamps_sec = sections_ini.setdefault("vamps", {})
        vamps_sec["logging"] = 0
        vamps_sec.pop("xtrasl", None)
        vamps_sec.pop("xtrapy", None)

        # Always use 1-based step numbering internally so the C code sees
        # firststep=1 (first step duration = 1 day).  self._firststep is the
        # calendar offset and is used only for BMI time reporting, not for
        # the C solver.  Passing a large calendar date (e.g. 33056) as
        # firststep would make the Richards solver try to take a 33056-day
        # first step and crash.
        self._ini_text = write_inp_str(
            sections_ini, "/dev/null", {}, firststep=1
        )

        # Map forcing names (VAMPS ts key) to BMI names for the variable catalogue
        self._input_var_names: tuple[str, ...] = tuple(
            _TS_TO_BMI[n] for n in self._forcing_names if n in _TS_TO_BMI
        )

        # Pre-allocate output buffers (filled by _collect_state)
        self._scalar_buf: dict[str, np.ndarray] = {
            bmi_name: np.zeros(self._ncols)
            for bmi_name in _SCALAR_OUTPUTS
        }
        # nlayers is unknown until after soil_init; set to 1 temporarily
        self._nlayers: int = 1
        self._profile_buf: dict[str, np.ndarray] = {}  # filled after init

        self._current_step: int = 0
        self._step_forcing: dict[str, np.ndarray] = {
            name: np.zeros(self._ncols) for name in self._forcing_names
        }

        if self._ncols == 1:
            self._init_single()
        else:
            self._init_multi()

        self._initialized = True

    def _init_single(self) -> None:
        """Single-column path: direct soil_init."""
        # Pre-allocate forcing arrays; C will hold pointers to these buffers.
        self._forcing_arrays: dict[str, np.ndarray] = {
            name: np.zeros(self._steps, dtype=np.float64)
            for name in self._forcing_names
        }
        _vampscore.soil_init(self._ini_text, self._forcing_arrays, 1.0)
        self._nlayers = _vampscore.soil_nlayers()
        self._profile_buf = {
            bmi_name: np.zeros((self._ncols, self._nlayers))
            for bmi_name in _PROFILE_OUTPUTS
        }

    def _init_multi(self) -> None:
        """Multi-column path: spawn one worker process per column."""
        ctx = mp.get_context("spawn")
        self._workers: list[mp.Process] = []
        self._cmd_queues:    list["mp.Queue[Any]"] = []
        self._result_queues: list["mp.Queue[Any]"] = []

        for col in range(self._ncols):
            cmd_q    = ctx.Queue()
            result_q = ctx.Queue()
            proc = ctx.Process(
                target=_column_worker,
                args=(
                    col, self._ini_text, self._steps, self._firststep,
                    self._forcing_names, cmd_q, result_q,
                ),
                daemon=True,
            )
            proc.start()
            self._workers.append(proc)
            self._cmd_queues.append(cmd_q)
            self._result_queues.append(result_q)

        # Collect 'ready' messages and determine nlayers
        nlayers_set: set[int] = set()
        for _ in range(self._ncols):
            msg = self._result_queues[_].get(timeout=30)
            if msg[0] != "ready":
                raise RuntimeError(f"Worker startup failed: {msg}")
            nlayers_set.add(msg[2])

        if len(nlayers_set) != 1:
            raise RuntimeError(f"Workers reported different nlayers: {nlayers_set}")
        self._nlayers = nlayers_set.pop()
        self._profile_buf = {
            bmi_name: np.zeros((self._ncols, self._nlayers))
            for bmi_name in _PROFILE_OUTPUTS
        }

    def update(self) -> None:
        """Advance the model by one timestep."""
        if not self._initialized:
            raise RuntimeError("Call initialize() first")
        if self._current_step >= self._steps:
            raise RuntimeError(
                f"Model has run all {self._steps} steps; call finalize() to reset"
            )

        step = self._current_step

        if self._ncols == 1:
            # Patch each forcing value directly into the C dataset array for
            # this step, then advance.  (ts_register_array copies values at
            # prelim() time, so writing into the numpy buffer after init has
            # no effect — we must use soil_patch_ts instead.)
            for ts_name in self._forcing_names:
                _vampscore.soil_patch_ts(ts_name, step,
                                         float(self._step_forcing[ts_name][0]))
            _vampscore.soil_step(step)
            state = _vampscore.soil_state(step)
            self._store_state(0, state)
        else:
            # Dispatch to all workers
            for col in range(self._ncols):
                forcing_update = {
                    ts_name: float(self._step_forcing[ts_name][col])
                    for ts_name in self._forcing_names
                }
                self._cmd_queues[col].put(("step", step, forcing_update))
            # Collect results (order may vary — use col_idx from message)
            for _ in range(self._ncols):
                msg = self._result_queues[_].get(timeout=60)
                if msg[0] != "state":
                    raise RuntimeError(f"Unexpected worker message: {msg}")
                _, col_idx, state = msg
                self._store_state(col_idx, state)

        self._current_step += 1

    def update_until(self, time: float) -> None:
        """Advance until model time reaches *time*."""
        end_step = int(round(time - self._firststep + 1))
        while self._current_step < end_step:
            self.update()

    def finalize(self) -> None:
        """Shut down workers and release resources."""
        if self._ncols > 1 and hasattr(self, "_cmd_queues"):
            for q in self._cmd_queues:
                q.put(("done",))
            for proc in self._workers:
                proc.join(timeout=5)
                if proc.is_alive():
                    proc.terminate()
        self._initialized = False

    # ------------------------------------------------------------------
    # Internal state helper
    # ------------------------------------------------------------------

    def _store_state(self, col: int, state: dict) -> None:
        """Write one column's state dict into the output buffers."""
        for bmi_name, vamps_key in _SCALAR_OUTPUTS.items():
            self._scalar_buf[bmi_name][col] = float(state.get(vamps_key, 0.0))
        for bmi_name, vamps_key in _PROFILE_OUTPUTS.items():
            arr = np.asarray(state.get(vamps_key, []), dtype=float)
            n = min(len(arr), self._nlayers)
            self._profile_buf[bmi_name][col, :n] = arr[:n]

    # ------------------------------------------------------------------
    # Model information
    # ------------------------------------------------------------------

    def get_component_name(self) -> str:
        return "VAMPS"

    def get_input_item_count(self) -> int:
        return len(self._input_var_names)

    def get_output_item_count(self) -> int:
        return len(_SCALAR_OUTPUTS) + len(_PROFILE_OUTPUTS)

    def get_input_var_names(self) -> tuple[str, ...]:
        return self._input_var_names

    def get_output_var_names(self) -> tuple[str, ...]:
        return tuple(_SCALAR_OUTPUTS) + tuple(_PROFILE_OUTPUTS)

    # ------------------------------------------------------------------
    # Variable information
    # ------------------------------------------------------------------

    def get_var_grid(self, name: str) -> int:
        if name in _PROFILE_OUTPUTS:
            return _GRID_PROFILE
        return _GRID_SCALAR

    def get_var_type(self, name: str) -> str:
        return "float64"

    def get_var_units(self, name: str) -> str:
        return _UNITS.get(name, "unknown")

    def get_var_itemsize(self, name: str) -> int:
        return np.dtype("float64").itemsize

    def get_var_nbytes(self, name: str) -> int:
        return self.get_var_itemsize(name) * self.get_grid_size(self.get_var_grid(name))

    def get_var_location(self, name: str) -> str:
        return "node"

    # ------------------------------------------------------------------
    # Time
    # ------------------------------------------------------------------

    def get_start_time(self) -> float:
        return float(self._firststep)

    def get_end_time(self) -> float:
        return float(self._firststep + self._steps - 1)

    def get_current_time(self) -> float:
        return float(self._firststep + self._current_step - 1)

    def get_time_step(self) -> float:
        return 1.0

    def get_time_units(self) -> str:
        return "d"

    # ------------------------------------------------------------------
    # Getters / setters
    # ------------------------------------------------------------------

    def get_value(self, name: str, dest: np.ndarray) -> np.ndarray:
        """Copy the current value of *name* into *dest* and return it."""
        src = self._get_buf(name).ravel()
        dest[:] = src
        return dest

    def get_value_ptr(self, name: str) -> np.ndarray:
        """Return a live reference to the internal buffer for *name*."""
        return self._get_buf(name)

    def get_value_at_indices(
        self, name: str, dest: np.ndarray, inds: np.ndarray
    ) -> np.ndarray:
        dest[:] = self._get_buf(name).ravel()[inds]
        return dest

    def set_value(self, name: str, src: np.ndarray) -> None:
        """Set forcing values for the next ``update()`` call.

        Parameters
        ----------
        name : str
            A BMI input variable name (e.g. ``'precipitation'``).
        src : ndarray
            Array of shape ``(ncols,)`` with one value per column.
        """
        ts_name = _BMI_TO_TS.get(name)
        if ts_name is None or ts_name not in self._forcing_names:
            raise KeyError(f"'{name}' is not a registered input variable")
        arr = np.asarray(src, dtype=float).ravel()
        if len(arr) == 1 and self._ncols > 1:
            arr = np.broadcast_to(arr, (self._ncols,)).copy()
        self._step_forcing[ts_name][:] = arr[: self._ncols]

    def set_value_at_indices(
        self, name: str, inds: np.ndarray, src: np.ndarray
    ) -> None:
        ts_name = _BMI_TO_TS.get(name)
        if ts_name is None or ts_name not in self._forcing_names:
            raise KeyError(f"'{name}' is not a registered input variable")
        self._step_forcing[ts_name][inds] = src

    def _get_buf(self, name: str) -> np.ndarray:
        if name in _SCALAR_OUTPUTS:
            return self._scalar_buf[name]
        if name in _PROFILE_OUTPUTS:
            return self._profile_buf[name]
        raise KeyError(f"Unknown BMI variable: '{name}'")

    # ------------------------------------------------------------------
    # Grid — Grid 0: scalar surface (ncols nodes)
    #        Grid 1: soil profile (ncols * nlayers nodes)
    # ------------------------------------------------------------------

    def get_grid_rank(self, grid: int) -> int:
        return 1 if grid == _GRID_SCALAR else 2

    def get_grid_size(self, grid: int) -> int:
        if grid == _GRID_SCALAR:
            return self._ncols
        return self._ncols * self._nlayers

    def get_grid_type(self, grid: int) -> str:
        if grid == _GRID_SCALAR:
            return "points"
        return "rectilinear"

    def get_grid_shape(self, grid: int, shape: np.ndarray) -> np.ndarray:
        if grid == _GRID_SCALAR:
            shape[0] = self._ncols
        else:
            shape[0] = self._ncols
            shape[1] = self._nlayers
        return shape

    def get_grid_spacing(self, grid: int, spacing: np.ndarray) -> np.ndarray:
        # Not meaningful for VAMPS (irregular layer thicknesses); return ones
        spacing[:] = 1.0
        return spacing

    def get_grid_origin(self, grid: int, origin: np.ndarray) -> np.ndarray:
        origin[:] = 0.0
        return origin

    def get_grid_x(self, grid: int, x: np.ndarray) -> np.ndarray:
        x[:] = np.arange(self._ncols, dtype=float)
        return x

    def get_grid_y(self, grid: int, y: np.ndarray) -> np.ndarray:
        if grid == _GRID_PROFILE:
            y[:] = np.arange(self._nlayers, dtype=float)
        return y

    def get_grid_z(self, grid: int, z: np.ndarray) -> np.ndarray:
        z[:] = 0.0
        return z

    def get_grid_node_count(self, grid: int) -> int:
        return self.get_grid_size(grid)

    def get_grid_edge_count(self, grid: int) -> int:
        return 0

    def get_grid_face_count(self, grid: int) -> int:
        return 0

    def get_grid_edge_nodes(self, grid: int, edge_nodes: np.ndarray) -> np.ndarray:
        return edge_nodes

    def get_grid_face_edges(self, grid: int, face_edges: np.ndarray) -> np.ndarray:
        return face_edges

    def get_grid_face_nodes(self, grid: int, face_nodes: np.ndarray) -> np.ndarray:
        return face_nodes

    def get_grid_nodes_per_face(
        self, grid: int, nodes_per_face: np.ndarray
    ) -> np.ndarray:
        return nodes_per_face
