# bmi_grid — Gridded BMI runner example

Drives a NY×NX grid of independent VAMPS soil columns through the
`VampsBmi` interface, streaming forcing from a NetCDF file and writing
all outputs to a second NetCDF file — one timestep at a time, no
full-run array kept in memory.

## Files

| File | Description |
|------|-------------|
| `make_forcing_nc.py` | Build a test forcing NetCDF from the Fiji `.prn` files |
| `run_bmi_grid.py` | BMI gridded runner |

## Quick start

```sh
cd examples/bmi_grid

# Step 1 — build a 4×5 gridded forcing file from the Fiji example data
python make_forcing_nc.py --ny 4 --nx 5 forcing.nc

# Step 2 — run the model
python run_bmi_grid.py forcing.nc --inp ../fiji/fiji.inp out.nc
```

## Forcing NetCDF format

The forcing file must have dimensions `(time, ny, nx)` and a `time`
variable whose values are the VAMPS timestep numbers (matching the `.inp`
forcing files).  One variable per BMI forcing name:

| Variable | Units |
|----------|-------|
| `precipitation` | cm d-1 |
| `net_radiation` | W m-2 |
| `incoming_radiation` | W m-2 |
| `relative_humidity` | 1 (0–1) |
| `air_temperature` | degC |
| `wind_speed` | m s-1 |

Variables absent from the file are silently skipped (the `.inp` defaults
apply).

## Output NetCDF structure

```
Dimensions: time(T), ny(NY), nx(NX), layer(NL)

Scalar outputs (time, ny, nx):
  soil_water_volume, soil_moisture_deficit,
  surface_runoff, bottom_drainage, avg_soil_water_content,
  transpiration_flux, soil_evaporation_flux, interception_flux

Profile outputs (time, ny, nx, layer):
  soil_water_content, pressure_head, hydraulic_conductivity
```

## Requirements

```sh
pip install netCDF4
```
