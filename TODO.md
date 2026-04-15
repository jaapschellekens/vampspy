# Wishlist / todo — VAMPS 1.0+

## Near term

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
