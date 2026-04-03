# Known bugs and missing features

## Solver

- Rutter interception not working correctly; use Gash (`method=0`) for daily steps
- Macro-pore flow not implemented (MPORE ifdefs are stubs)
- `calcgwl` may give incorrect results in some configurations; groundwater
  table output should be verified against independent estimates

## Python / vampspy

- `vamps_do_step_direct()` bypasses the canopy (`tstep_top`); when replaying
  captured post-canopy fluxes the results may diverge slightly from
  `vamps_do_step()` due to internal canopy state not being reproduced exactly
- `_vampscore` functions are not thread-safe (C globals); use `multiprocessing`,
  not `threading`, for parallel runs (`Model.run_grid` does this correctly)
- `soil_state_current()` after a `run_grid` probe run consumes one extra
  `soil_init` call in the parent process; this is a known minor inefficiency

## General

- More than one space after a filename in the `[ts]` section is not stripped
  (the C parser is strict about whitespace)
- Crash if no `[determine]` variable is set in the config
