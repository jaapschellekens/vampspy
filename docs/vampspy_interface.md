# vampspy — Python Interface to VAMPS

VAMPS (VAriably saturated soil Model with Plant and canopy System) is a 1D
unsaturated zone model.  The `vampspy` package provides two ways to drive it
from Python:

| Interface | How it works | When to use |
|-----------|-------------|-------------|
| `vampspy._vampscore.run()` | C extension, **in-process**, no files | Fast repeated runs, parameter studies |
| `vampspy.Model.run()` | High-level class; uses C extension when available, subprocess otherwise | General use, portability |
| `vampspy.Model.run_stepwise()` | Python drives the outer timestep loop | Inspect or modify state between steps |
| `vampspy.Model.run_grid()` | Parallel multi-column / 2-D grid run | Spatial grids, batch parameter sweeps |

---

## Installation

```bash
# From the repo root — builds the C extension in-place
pip install -e .
```

`./configure` must have been run in `src/` first (produces `src/vconfig.h`).

---

## 1. Low-level API — `vampspy._vampscore.run()`

### Signature

```python
result = vampspy._vampscore.run(config_file, forcing, firststep=1.0)
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `config_file` | `str` | Path to a VAMPS `.inp` configuration file |
| `forcing` | `dict[str, np.ndarray]` | Forcing time series — one 1-D `float64` array per `[ts]` variable |
| `firststep` | `float` | X-coordinate of the first forcing element (default `1.0`) |

### How forcing override works

The `.inp` file contains a `[ts]` section listing forcing variable names and
their data files, e.g.:

```ini
[ts]
pre = precip.prn
nra = rnet.prn
tem = newt.prn
```

Any variable whose **name** matches a key in `forcing` is served from the
supplied numpy array instead of opening a file.  Variables not present in
`forcing` are still read from disk (relative to the working directory).  This
means you can supply some variables as arrays and leave others as files.

### Return value

A `dict` with the following numpy arrays:

#### Scalar time series — shape `(steps,)`

| Key | Units | Description |
|-----|-------|-------------|
| `t` | — | Timestep x-values (as in the forcing files) |
| `volact` | cm | Actual soil water volume (integrated over profile) |
| `SMD` | cm | Soil moisture deficit |
| `avgtheta` | — | Average volumetric water content over profile |
| `qtop` | cm/timestep | Flux through the top of the soil profile |
| `qbot` | cm/timestep | Flux through the bottom of the soil profile |
| `cumprec` | cm | Cumulative precipitation |
| `cumtra` | cm | Cumulative transpiration |
| `cumeva` | cm | Cumulative soil evaporation |
| `cumintc` | cm | Cumulative canopy interception |
| `masbal` | cm | Mass balance error |
| `precipitation` | cm/timestep | Precipitation at each step (from forcing) |
| `interception` | cm/timestep | Canopy interception at each step |
| `transpiration` | cm/timestep | Transpiration at each step |
| `soilevaporation` | cm/timestep | Soil evaporation at each step |

#### Profile time series — shape `(steps, nlayers)`

| Key | Units | Description |
|-----|-------|-------------|
| `theta` | m³/m³ | Volumetric water content per soil layer per timestep |

`nlayers` equals the number of soil layers defined in the `.inp` file (e.g. 77
for the fiji example).

### Errors

| Exception | Cause |
|-----------|-------|
| `ValueError` | `forcing` dict is empty, or an array has the wrong dtype |
| `RuntimeError` | VAMPS internal error (e.g. missing config section) |

### Example — direct use with an existing `.inp` file

```python
import os
import numpy as np
from vampspy import _vampscore

DATA = "examples/fiji"
os.chdir(DATA)          # so relative paths in fiji.inp resolve

def load(fname, col=1):
    return np.loadtxt(fname, usecols=col)

forcing = {
    "pre": load("precip.prn"),
    "nra": load("rnet.prn"),
    "ira": load("inrad.prn"),
    "rlh": load("rh.prn"),
    "tem": load("newt.prn"),
    "win": load("wind.prn"),
}

result = _vampscore.run("fiji.inp", forcing, firststep=1.0)

print(result["volact"][-1])      # final water storage [cm]
print(result["theta"].shape)     # (61, 77)
```

### Repeated runs

All C-level global state is reset between calls, so consecutive calls give
bit-for-bit identical results:

```python
r1 = _vampscore.run("fiji.inp", forcing)
r2 = _vampscore.run("fiji.inp", forcing)
assert (r1["volact"] == r2["volact"]).all()   # always True
```

State that is reset on each call:

- Dataset registry (`del_all_sets`)
- Soil firsttime flag → fresh allocation and parameter read (`reset_presoil`)
- Cumulative totals — volact, cumprec, etc. (`settotzero`)
- Surface ponding (`pond = 0`)
- Soil type counter (`spnr = 0`)
- Canopy wetness state — lastwet, wetsteps, canopy storage, ea/es/VPD
  (`reset_canopy`)
- Adaptive timestep — dt, dtm1 (`reset_timestep`)

---

## 2. High-level API — `vampspy.Model`

`Model` accepts the configuration as a Python dict (mirroring the `.inp` INI
format) rather than requiring a file.  It automatically writes a temporary
`.inp`, runs the solver, and returns the same result dict.

When the C extension is available it runs in-process (Phase 2).  Otherwise it
falls back to spawning the `vamps` binary and parsing the `.out` file (Phase 1).

### Constructor

```python
model = vampspy.Model(config, forcing, executable="vamps", vampslib=None)
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `config` | `dict` | Nested dict — `{section: {key: value}}` — mirroring `.inp` sections |
| `forcing` | `dict[str, array-like]` | Forcing time series (same keys as `[ts]` section) |
| `executable` | `str` | Path to the `vamps` binary (Phase 1 fallback only). Overrides `VAMPS_BINARY` env var |
| `vampslib` | `str` | Path to the VAMPS share directory (Phase 1 only). Overrides `VAMPSLIB` env var |

All forcing arrays must have the same length; a `ValueError` is raised
otherwise.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `model.steps` | `int` | Number of timesteps, inferred from the forcing array lengths |

### `Model.run()`

```python
result = model.run(firststep=1.0)
```

Returns the same dict as `_vampscore.run()` (same keys, same shapes).

The Phase 1 fallback additionally provides:

| Key | Description |
|-----|-------------|
| `_steps` | Number of timesteps parsed from the `.out` file |
| `_initial` | Dict of values from the `[initial]` section of the `.out` file |
| `_t` | Alias for `t` |

### Example

```python
import numpy as np
from vampspy import Model

config = {
    "determine": {"soilmoisture": 1},
    "top":       {"system": 5},           # Penman-Monteith canopy
    "interception": {
        "method":  0,                     # Gash
        "E_avg/R": 0.147,
        "p_f":     0.6,
        "p_tr":    0.017,
        "S":       0.08,
    },
    "canopy": {
        "Rnet_absorb": 0.975,
        "layers": 1,
        "z":  12.7, "z_0": 1.5, "d": 7.0, "rs": 60,
    },
    "soil": {
        "layers":   77,
        "initprof": 0,
        "bottom":   6,
        "mktable":  1,
        "theta_initial": [...],           # list of 77 values
    },
    "layer_0":  {"thickness": 2.0, "soilsection": "st_0"},
    "st_0": {
        "method": 1,
        "thetas": 0.6, "theta_residual": 0.08,
        "alpha":  0.061, "n": 1.098, "ksat": 1800,
    },
    # ... additional layer and soil-type sections
}

forcing = {
    "pre": precip_array,    # precipitation [cm/day], shape (steps,)
    "nra": rnet_array,
    "ira": inrad_array,
    "rlh": rh_array,
    "tem": temp_array,
    "win": wind_array,
}

model = Model(config, forcing)
result = model.run()

print(f"Final volact: {result['volact'][-1]:.3f} cm")
print(f"Theta shape:  {result['theta'].shape}")
```

### `Model.run_stepwise()`

```python
result = model.run_stepwise(firststep=1.0)
```

Python owns the outer timestep loop.  For each step the method calls
`_vampscore.soil_step(i)` (C canopy + Richards solver) then collects state
via `_vampscore.soil_state_current()`.  The result dict is identical in
structure to `run()`.

Use this when you need to inspect or modify simulation state between external
timesteps — e.g. to implement lateral redistribution between soil columns or
to feed model output back into the forcing of the next step.

Requires the `_vampscore` C extension.

### `Model.run_grid()`

```python
result = model.run_grid(
    forcing_grid,           # dict[str, ndarray] — values shape (ncols, steps)
                            #   or (ny, nx, steps) for a 2-D geographic grid
    nworkers=None,          # int — worker processes (default: os.cpu_count())
    chunk_size=None,        # int — columns per task (default: ncols//(nworkers*4))
    firststep=1.0,
)
```

Runs the 1-D solver for many independent soil columns in parallel using
`multiprocessing` with POSIX shared memory.  All columns share the same
soil profile configuration (`self.config`); each receives its own forcing
time series.

**Data flow:**

1. Forcing is packed once into a shared-memory block `(nvars, ncols, steps)`.
   Workers read column slices as numpy views — no IPC copying.
2. Result blocks (scalars, profiles) are pre-allocated in shared memory.
   Workers write directly into them.
3. After all workers finish the parent copies results to ordinary numpy arrays
   and closes/unlinks the shared-memory blocks.

**Return value** — same keys as `run()`, with spatial dimensions prepended:

| Key | Shape (2-D grid example) |
|-----|--------------------------|
| Scalars (`volact`, `SMD`, …) | `(ny, nx, steps)` |
| Profiles (`theta`, `k`, `h`, …) | `(ny, nx, steps, nlayers)` |
| `q`, `inq` | `(ny, nx, steps, nlayers+1)` |
| `gwl` | `(ny, nx, steps, 2)` |
| `_steps`, `_nlayers` | `int` |

```python
import numpy as np
from vampspy.model import Model

m = Model.from_file('examples/fiji/fiji.inp', vampslib='share')

NY, NX = 25, 40   # 1000-cell grid
forcing_grid = {k: np.tile(v, (NY, NX, 1)) for k, v in m.forcing.items()}

result = m.run_grid(forcing_grid, nworkers=4)
print(result['volact'].shape)   # (25, 40, 61)
print(result['theta'].shape)    # (25, 40, 61, 77)
```

See `notebooks/grid_1000_cells.ipynb` for a full worked example with spatially
varied precipitation, spatial maps, time-series comparisons, and scaling
benchmarks.

---

## 3. Low-level stepwise API — `_vampscore`

These functions are used internally by `Model.run_stepwise()` but can also be
called directly for maximum control.

```python
from vampspy import _vampscore

# Initialise once
_vampscore.soil_init(ini_text, forcing, firststep)
nlayers = _vampscore.soil_nlayers()

# Python timestep loop
for i in range(steps):
    _vampscore.soil_step(i)           # C canopy + Richards solver
    s = _vampscore.soil_state_current()  # → dict of scalars + profiles
    # s keys: t, volact, SMD, qtop, qbot, avgtheta,
    #         cumprec, cumtra, cumeva, cumintc, masbal,
    #         precipitation, interception, transpiration, soilevaporation,
    #         theta, k, h, q, inq, qrot, howsat, gwl, _nlayers
```

### `soil_step_direct(pre, intc=0.0, ptra=0.0, peva=0.0, rdp=0.0)`

Like `soil_step(i)` but accepts per-step forcing scalars directly instead of
reading from the registered forcing arrays.  **Bypasses the C canopy
(`tstep_top`) — the caller is responsible for any canopy calculation.**

```python
_vampscore.soil_step_direct(
    pre=0.5,    # net precipitation at soil surface [cm/day]
    intc=0.0,   # interception [cm/day]
    ptra=0.3,   # potential transpiration [cm/day]
    peva=0.05,  # potential soil evaporation [cm/day]
    rdp=120.0,  # root depth [cm]
)
s = _vampscore.soil_state_current()
```

### `soil_state_current()`

Returns the state after the most recently completed `soil_step()` or
`soil_step_direct()` call.  No step index is required.

### `soil_get_profiles()`  (via `soil_state_current`)

Profile arrays are included in the `soil_state_current()` return dict:

| Key | Shape | Description |
|-----|-------|-------------|
| `theta` | `(nlayers,)` | Volumetric water content |
| `k` | `(nlayers,)` | Hydraulic conductivity |
| `h` | `(nlayers,)` | Pressure head |
| `qrot` | `(nlayers,)` | Root water uptake |
| `howsat` | `(nlayers,)` | Degree of saturation |
| `q` | `(nlayers+1,)` | Inter-layer flux |
| `inq` | `(nlayers+1,)` | Cumulative inter-layer flux |
| `gwl` | `(2,)` | Groundwater table levels |

---

## 5. Forcing variables

The following variable names are standard in VAMPS.  Not all need to be
supplied — only those referenced in the `.inp` configuration:

| Name | Description | Typical unit |
|------|-------------|-------------|
| `pre` | Precipitation | cm/timestep |
| `nra` | Net radiation | W/m² |
| `ira` | Incoming (global) radiation | W/m² |
| `rlh` | Relative humidity | fraction (0–1) |
| `tem` | Air temperature | °C |
| `win` | Wind speed | m/s |
| `ptr` | Potential transpiration (pre-computed) | cm/timestep |
| `spe` | Potential soil evaporation (pre-computed) | cm/timestep |
| `inr` | Interception (pre-computed) | cm/timestep |
| `hea` | Pressure head at the bottom boundary | cm |
| `rdp` | Rooting depth | cm |
| `gwt` | Groundwater table depth | cm |
| `lai` | Leaf area index | m²/m² |

---

## 6. Configuration (`config` dict / `.inp` file)

The configuration file uses an INI-style format with section headers
`[section_name]` and `key = value` pairs.  Arrays are written as
space-separated values with optional backslash line continuation.

Key sections:

| Section | Purpose |
|---------|---------|
| `[run]` | Output file path, misc run flags |
| `[time]` | `steps`, `firststep`, `startpos` |
| `[ts]` | Forcing variable names → file paths |
| `[determine]` | Toggle `soilmoisture`, `canopy` computation |
| `[top]` | Canopy top-system selection (`system = 5` for Penman-Monteith) |
| `[canopy]` | Canopy geometry, aerodynamic/stomatal resistance |
| `[interception]` | Interception method and Gash/Rutter parameters |
| `[soil]` | Soil solver settings: layers, bottom BC, numerical tolerances |
| `[layer_N]` | Per-layer settings (thickness, soil section reference) |
| `[st_N]` | Soil type hydraulic parameters (Van Genuchten or Brooks-Corey) |
| `[roots]` | Root extraction function parameters |

---

## 7. Building from source

```bash
cd /path/to/vamps/src
./configure           # generates vconfig.h
cd ..
pip install -e .      # builds vampspy._vampscore in-place
```

The `setup.py` compiles the entire VAMPS C source tree (≈ 60 files) with
`-DVAMPS_EXT_BUILD` which suppresses the standalone `main()` and
`dotail()` functions.

### Requirements

- Python ≥ 3.9
- NumPy (any recent version)
- A C compiler (clang or gcc)

---

## 8. Thread safety

`vampspy._vampscore.run()` is **not thread-safe**.  VAMPS uses process-wide C
globals.  If you need parallelism, use `multiprocessing` (separate processes
each have their own copy of the globals):

```python
from multiprocessing import Pool
from vampspy import _vampscore

def run_one(args):
    cfg, forcing = args
    return _vampscore.run(cfg, forcing)

with Pool(4) as p:
    results = p.map(run_one, param_list)
```
