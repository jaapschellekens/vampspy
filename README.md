# VAMPS — Vegetation-AtMosPhere-Soil water model

**Version 1.0** — J. Schellekens &lt;jaap@ws-fitzcarraldo.nl&gt;

VAMPS is a one-dimensional model for the flow of water through a forested
or agricultural ecosystem.  The core is a Richards-equation solver for
unsaturated flow through the soil, derived from the SWAP94 model.  It
supports arbitrarily sized and variable timesteps — for example one year of
daily steps with selected days at 15-minute resolution.

The canopy water balance (interception, transpiration, soil evaporation) is
handled by the topsys module.  Several canopy modes are available, ranging
from bare-soil to full Penman-Monteith with Gash interception.

Since version 1.0 the scripting engine is Python (CPython embedded in the
binary).  Users can hook custom functions into a run via `[vamps] xtrapy`
in the configuration file.  S-Lang, used in versions prior to 1.0, has been
removed.

---


## vampspy — Python API

The `vampspy` package lets you drive VAMPS entirely from Python with no
subprocess or temporary files:

| Method | Description |
|--------|-------------|
| `Model.from_file(path)` | Load a `.inp` config file |
| `Model(config, forcing)` | Build a model from Python dicts and numpy arrays |
| `model.run()` | Whole-run — C drives the timestep loop |
| `model.run_stepwise()` | Python drives the outer timestep loop; inspect or modify state between steps |
| `model.run_grid(forcing_grid)` | Parallel multi-column / 2-D grid run; accepts `(ncols, steps)` or `(ny, nx, steps)` forcing arrays |

```python
from vampspy.model import Model

m = Model.from_file('examples/fiji/fiji.inp', vampslib='share')
result = m.run()
print(result['volact'][-1])   # final water storage [cm]

# 25×40 grid run
import numpy as np
fg = {k: np.tile(v, (25, 40, 1)) for k, v in m.forcing.items()}
result = m.run_grid(fg, nworkers=4)
print(result['volact'].shape)  # (25, 40, 61)
```

Build and install:

```sh
cd src && ./configure
pip install -e .
export VAMPSLIB=/path/to/vamps/share
```

See [docs/vampspy_interface.md](docs/vampspy_interface.md) for the full API
and [notebooks/](notebooks/) for worked examples including a 1000-cell grid run.

---

## Distribution

Vamps is distributed with full source code.  Tested on:

- macOS (arm64, x86_64) with Apple clang
- Linux (x86_64, arm64) with gcc

Source: <https://github.com/jaapschellekens/vampspy>

---

## System requirements

- C compiler: gcc or clang (C99)
- GNU make
- Python 3.9 or later (with development headers; `python3-config` on PATH)
- NumPy

termcap or ncurses is required only for the interactive binary mode.

---

## Quick start

```sh
cd src && ./configure && make all
export VAMPSLIB=/path/to/vamps/share
cd ../examples/fiji && vamps fiji.inp
```

---

## Feedback

Bug reports and comments: <https://github.com/jaapschellekens/vampspy/issues>
