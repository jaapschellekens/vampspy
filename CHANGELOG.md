# Changelog

## Version 1.0 (April 2026) — vampspy Python layer

- Removed S-Lang scripting engine; replaced with embedded CPython
  (`init_py.c`, `vamps_ext.c`). `[vamps] xtrapy = myscript.py` hooks
  `at_start` / `each_step` / `at_end` Python functions into runs.
- Built `vampspy` Python package with `_vampscore` C extension for
  in-process (no subprocess, no temp files) VAMPS runs:
  - `_vampscore.run(ini_text, forcing, firststep)`
  - `_vampscore.soil_init` / `soil_step` / `soil_state` / `soil_nlayers`
  - `_vampscore.soil_step_direct` / `soil_state_current` / `soil_get_profiles`
- `soil_api.c`: per-step C API (`vamps_init_stepwise_str`, `vamps_do_step`,
  `vamps_get_state`, `vamps_get_profiles`, `vamps_do_step_direct`,
  `vamps_get_state_current`)
- `vampspy.Model`: high-level Python class
  - `Model.from_file()` — parses `.inp` and loads all forcing time series
  - `model.run()` — whole-run, C-driven loop
  - `model.run_stepwise()` — Python-driven outer timestep loop
  - `model.run_grid()` — parallel multi-column / 2-D grid run using
    `multiprocessing` + POSIX shared memory; accepts `(ncols, steps)` or
    `(ny, nx, steps)` forcing arrays; results reshaped to match
- `vampspy._grid`: shared-memory parallel worker pool — forcing packed once
  into shared memory (zero IPC copy); workers attach by name and write
  results back. Tested: 1000-cell 25×40 grid, 61 steps, 77 layers ≈ 10 s on 4 cores.
- Removed all RCS `*,v` files and `RCS/` directories (154 files)
- Removed unused `#include "marquard.h"` from 7 source files
- Repo: <https://github.com/jaapschellekens/vampspy>

## Version 1.0 (March 2017)

- Tagged as 1.0 after long period of stability at 0.99i

## Version 0.99i (November 2001)

- vampsfit additions
- Updated address and version information

## Version 0.99h (February 2000)

- Added slang 0.99.38 to distribution

## Version 0.99g (January 1999)

- Bug fixes to cumulative totals (`_cumtra` etc.)

## Version 0.99f (November 1997)

- Started `nhc.c` — rewritten soil moisture module using TOPOG approach
- Optimised `getval` with hunting phase (~20% faster)
- Fixed bug in `ra` interface in `met.lib`

## Version 0.99e

- `configure` now generates Makefiles and `vconfig.h`
- Added AGL graphics library S-Lang interface
- Added gnuplot graphics interface

## Version 0.99d

- Added S-Lang based keypressed check in main loop

## Version 0.99c (May 1997 snapshot)

- Removed goto labels from `headcalc.c`
- Added `filename,xcol,ycol` column selection in `[ts]` section
- Checked bottom conditions 1 and 4
- Added new mass-balance convergence check (`[soil] mbck = 1`)
- Major refactor of `deffile.lib` — less memory, multiple files in memory

## Version 0.99b (December 1996)

- Added `sp` struct for soil-specific info; renamed `soilparm` to `node`
- Input file changed: soil-specific parameters now in separate `[st_N]` sections
- Added S-Lang `theta`/`head`/`dmc`/`k_unsat` function support

## Version 0.99 (August 1996)

- Added S-Lang scripting support (`at_start`, `each_step`, `at_end`)
- Added `-F` option for fully interactive mode

## Version 0.98 (March 1996)

- Sub-daily timesteps working; removed `-DTRY_SUBDAY` requirement
- Added band-diagonal solver for zero-pivot cases
- Added saturation handling

## Version 0.97 (February 1996)

- Support for TOPOG soil tables
- Timestep now determined from x-column in precipitation file

## Version 0.95–0.96 (December 1995)

- Added look-up tables option (up to 50% speedup)

## Version 0.1 (January 1995)

- Initial framework
