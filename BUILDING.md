# Building VAMPS

## Prerequisites

### Required

- C compiler: `gcc` or `clang` (C99)
- `make`
- Python 3.9 or later with development headers (`python3-config` must be on `PATH`)
- NumPy (`pip install numpy`) — required for the `vampspy` Python package
- `termcap` or compatible library (`-ltermcap`) — required only for interactive binary mode

On macOS with Homebrew:

```sh
brew install python
```

On Debian/Ubuntu:

```sh
apt install gcc make python3-dev
```

### No longer required

S-Lang has been removed. The scripting engine is now CPython, embedded directly into the binary.

---

## Build steps

All build commands are run from the `src/` directory.

### 1. Build everything

```sh
cd src
make all
```

`make all` automatically builds the four static libraries first (into `libDarwin_arm64/` or the OS-appropriate sibling directory), then compiles and links the binaries:

| Library | Source directory |
|---|---|
| `libdef.a` | `deffile.lib/` |
| `libnru.a` | `nr_ut.lib/` |
| `libts.a` | `ts.lib/` |
| `libmet.a` | `met.lib/` |

This produces two executables in `src/`:

- `vamps` — the main simulation binary
- `vsel` — utility for extracting time-series variables from `.out` files

The Python libraries are discovered at build time via `python3-config --embed --ldflags`. If you have multiple Python versions installed, set `PATH` accordingly before running `make`.

---

## Platform notes

The build output directory is named after the OS and architecture (e.g. `objsDarwin_arm64/`, `libDarwin_arm64/`). The Makefiles detect this via the `OS` variable in `src/Makefile`. If you are building on a different platform you may need to adjust this variable and the corresponding directory names in the sub-Makefiles.

---

## Python runtime environment

VAMPS loads a startup script and user scripts via the embedded Python interpreter. Set `VAMPSLIB` to the directory containing the Python library files before running:

```sh
export VAMPSLIB=/usr/local/share/vamps1.0   # installed location
# or, from the source tree:
export VAMPSLIB=/path/to/vamps/share
```

The following Python library modules are provided in `share/`:

| Module | Purpose |
|---|---|
| `vamps_startup.py` | Auto-loaded before every user script; provides defaults and adds `$VAMPSLIB` to `sys.path` |
| `util.py` | `.out` file parser, `vsel()`, `vprof()`, `f_save()` |
| `regress.py` | Surface resistance regression (`estrs`) |
| `met.py` | Meteorological functions (Penman–Monteith helpers) |
| `runut.py` | Run utilities: `savets`, `showprof`, `saveprof` |
| `soilf.py` | Clapp/Hornberger soil model (soil `method = 5`) |
| `topsys.py` | Scripted top-system (top `system = 6`) |
| `stop.py` | Mass-balance check helper |
| `stats.py` | Column statistics: `mean`, `sdev`, `linreg`, etc. |
| `rep.py` | Human-readable report generator for `.out` files |
| `cat.py`, `sec.py`, `vmany.py`, `vsel.py`, `site.py` | Minor utilities |

---

## Hooking a Python script into a run

Add to the `[vamps]` section of your `.inp` file:

```ini
[vamps]
xtrapy = myscript.py
```

The path is resolved relative to the `.inp` file. The script may define any combination of:

```python
def at_start():   ...   # called once before the first timestep
def each_step():  ...   # called after every timestep
def at_end():     ...   # called after the last timestep
```

Import `vamps` to access simulation state:

```python
import vamps

def at_end():
    vamps.printsum()
    print(f"CPU: {vamps.cpu():.3f}s")
```

---

## Building and installing the vampspy Python package

`vampspy` is the Python API for VAMPS.  It compiles the full C source tree
into a Python extension module (`_vampscore.so`) with `-DVAMPS_EXT_BUILD`,
which suppresses the standalone `main()` and `dotail()` functions.

```sh
# From the repo root (after ./configure in src/)
pip install -e .
```

This produces `vampspy/_vampscore.cpython-*.so` in place.

### Using vampspy

```python
from vampspy.model import Model

m = Model.from_file('examples/fiji/fiji.inp', vampslib='share')

# Whole-run (C drives the loop)
result = m.run()

# Python drives the outer timestep loop
result = m.run_stepwise()

# Parallel 2-D grid run — accepts (ny, nx, steps) forcing arrays
import numpy as np
forcing_grid = {k: np.tile(v, (25, 40, 1)) for k, v in m.forcing.items()}
result = m.run_grid(forcing_grid, nworkers=4)
# result['volact'].shape → (25, 40, 61)
# result['theta'].shape  → (25, 40, 61, nlayers)
```

See `docs/vampspy_interface.md` for the full API reference and `notebooks/`
for worked examples including a 1000-cell grid run.

---

## Running the examples

```sh
export VAMPSLIB=/path/to/vamps/share

# Binary
cd examples/fiji && vamps fiji.inp

# Python
cd examples/fiji && python fiji.py

# Notebook
cd notebooks && jupyter notebook grid_1000_cells.ipynb
```

---

## Clean

```sh
# Remove object files only
make -C src clean

# Remove object files and libraries
make -C src distclean
```
