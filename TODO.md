# Wishlist / todo — VAMPS 1.0+

## Near term

- Move C solver globals into a per-column context struct (`vamps_col_t`) so
  that multiple independent columns can run in the same process without
  multiprocessing overhead.  This is the main blocker for a vectorised
  Python loop over grid cells.

- Investigate `vamps_do_step_direct` canopy divergence: determine whether
  capturing `tstep_top` outputs and replaying via `soil_step_direct`
  is sufficient for full canopy bypass, or whether `tstep_top` internal
  state (canopy storage) also needs to be saved/restored.

- Add `soil_init` / `soil_step` / `soil_state` to `Model.run_stepwise`
  docstring with a note about which state persists between steps.

- Silence the C progress table by default when called from Python;
  add a `verbose=True` parameter to `run()`, `run_stepwise()`, `run_grid()`.

- Test suite: add pytest-based regression tests for all three execution paths.

## Longer term

- Working macro-pore setup (MPORE stubs in `soil/`)
- Litter interception layer above canopy
- Multi-layer canopy (`canopy.c` scaffolding exists but is incomplete)
- Priestly-Taylor evapotranspiration option in `met.lib`
- Lateral flow between grid cells (currently each column is independent)
- NetCDF / Zarr output for grid runs
