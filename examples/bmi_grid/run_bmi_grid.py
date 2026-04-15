"""
run_bmi_grid.py — Gridded BMI runner for VAMPS.

Drives a NY×NX grid of independent VAMPS soil columns, streaming forcing
from a NetCDF file one timestep at a time and writing all outputs to a
second NetCDF file one timestep at a time — no full-run array is held in
memory.

Usage
-----
    python run_bmi_grid.py forcing.nc --inp model.inp [out.nc]

    forcing.nc   NetCDF4 file with dimensions (time, ny, nx) and one
                 variable per BMI forcing name (see below).  The ``time``
                 variable supplies the timestep values and implicitly
                 defines nsteps and firststep.
    --inp        VAMPS .inp configuration file (soil, canopy, roots …).
    out.nc       Output file (default: out.nc).

Forcing variable names expected in forcing.nc
---------------------------------------------
    precipitation       cm d-1
    net_radiation       W m-2
    incoming_radiation  W m-2
    relative_humidity   1  (0–1)
    air_temperature     degC
    wind_speed          m s-1

Requirements
------------
    pip install netCDF4
"""
from __future__ import annotations

import argparse
import os
import sys
import tempfile

import numpy as np
import yaml

try:
    import netCDF4 as nc
except ImportError:
    sys.exit("netCDF4 is required:  pip install netCDF4")

from vampspy.bmi import VampsBmi


def _bmi_from_nc(forcing_path: str, inp_file: str) -> "tuple[VampsBmi, int, int, int]":
    """Open the forcing file, build a VampsBmi and return (bmi, ny, nx, nsteps).

    A minimal YAML config is generated from the NetCDF metadata and passed
    to VampsBmi.initialize().  The user never needs to write or maintain a
    YAML file.
    """
    ds = nc.Dataset(forcing_path, "r")
    ny      = len(ds.dimensions["ny"])
    nx      = len(ds.dimensions["nx"])
    t       = np.asarray(ds["time"][:], dtype=float)
    nsteps  = len(t)
    firststep = float(t[0])
    ds.close()

    # Build a minimal config dict; inp_file must be absolute so the YAML
    # can live in the system temp directory without breaking relative paths.
    cfg = {
        "inp_file":  os.path.abspath(inp_file),
        "ncols":     ny * nx,
        "steps":     nsteps,
        "firststep": firststep,
    }

    with tempfile.NamedTemporaryFile(
        mode="w", suffix=".yml", delete=False
    ) as tf:
        yaml.dump(cfg, tf)
        tmp_yml = tf.name

    bmi = VampsBmi()
    try:
        bmi.initialize(tmp_yml)
    finally:
        os.unlink(tmp_yml)

    return bmi, ny, nx, nsteps


def _create_output(path: str, ny: int, nx: int, nlayers: int,
                   bmi: VampsBmi) -> "tuple[nc.Dataset, dict, dict]":
    """Create the output NetCDF4 file and return (dataset, scalar_vars, profile_vars)."""
    ds = nc.Dataset(path, "w", format="NETCDF4")
    ds.Conventions = "CF-1.8"
    ds.title       = "VAMPS BMI gridded output"
    ds.source      = "run_bmi_grid.py"

    ds.createDimension("time",  None)   # unlimited
    ds.createDimension("ny",    ny)
    ds.createDimension("nx",    nx)
    ds.createDimension("layer", nlayers)

    vt            = ds.createVariable("time", "f8", ("time",))
    vt.units      = bmi.get_time_units()
    vt.long_name  = "model time"

    scalar_names = [
        "soil_water_volume", "soil_moisture_deficit",
        "surface_runoff", "bottom_drainage",
        "avg_soil_water_content",
        "transpiration_flux", "soil_evaporation_flux", "interception_flux",
    ]
    profile_names = [
        "soil_water_content", "pressure_head", "hydraulic_conductivity",
    ]

    scalar_vars: dict[str, nc.Variable] = {}
    for name in scalar_names:
        v = ds.createVariable(name, "f4", ("time", "ny", "nx"),
                              zlib=True, complevel=4)
        v.units = bmi.get_var_units(name)
        scalar_vars[name] = v

    profile_vars: dict[str, nc.Variable] = {}
    for name in profile_names:
        v = ds.createVariable(name, "f4", ("time", "ny", "nx", "layer"),
                              zlib=True, complevel=4)
        v.units = bmi.get_var_units(name)
        profile_vars[name] = v

    return ds, scalar_vars, profile_vars


def run(forcing_path: str, inp_file: str, out_path: str) -> None:
    bmi, ny, nx, nsteps = _bmi_from_nc(forcing_path, inp_file)
    ncols   = ny * nx
    nlayers = bmi.get_grid_size(1) // ncols

    print(f"VAMPS BMI gridded runner")
    print(f"  forcing : {forcing_path}  ({nsteps} steps, {ny}×{nx} grid)")
    print(f"  model   : {inp_file}  ({nlayers} layers)")
    print(f"  output  : {out_path}")

    # Open forcing file for per-timestep reads
    frc = nc.Dataset(forcing_path, "r")
    active_inputs = set(bmi.get_input_var_names())

    ds, scalar_vars, profile_vars = _create_output(out_path, ny, nx, nlayers, bmi)

    dest_scalar  = np.zeros(ncols,          dtype=np.float64)
    dest_profile = np.zeros(ncols * nlayers, dtype=np.float64)

    print("  Running...")
    for i in range(nsteps):
        t = bmi.get_current_time()

        # Stream one timestep of forcing into the BMI
        for name in active_inputs:
            if name in frc.variables:
                vals = np.asarray(frc[name][i], dtype=np.float64).ravel()
                bmi.set_value(name, vals)

        bmi.update()

        ds["time"][i] = t

        mean_volact = 0.0
        for name, ncvar in scalar_vars.items():
            bmi.get_value(name, dest_scalar)
            ncvar[i, :, :] = dest_scalar.reshape(ny, nx).astype("f4")
            if name == "soil_water_volume":
                mean_volact = float(dest_scalar.mean())

        for name, ncvar in profile_vars.items():
            bmi.get_value(name, dest_profile)
            ncvar[i, :, :, :] = dest_profile.reshape(ny, nx, nlayers).astype("f4")

        if i == 0 or (i + 1) % 10 == 0 or i == nsteps - 1:
            print(f"    step {i+1:3d}/{nsteps}  t={t:.0f}  "
                  f"mean volact={mean_volact:.3f} cm")

    frc.close()
    ds.close()
    bmi.finalize()

    print(f"\nDone.  Output written to: {out_path}")
    _print_summary(out_path)


def _print_summary(path: str) -> None:
    ds = nc.Dataset(path, "r")
    volact = np.asarray(ds["soil_water_volume"][:])
    theta  = np.asarray(ds["soil_water_content"][:])
    ny, nx = volact.shape[1], volact.shape[2]
    print(f"\n--- Output summary ---")
    print(f"  Dimensions: time={volact.shape[0]}, ny={ny}, nx={nx}, "
          f"layer={theta.shape[3]}")
    print(f"  soil_water_volume : "
          f"min={volact.min():.3f}  mean={volact.mean():.3f}  max={volact.max():.3f} cm")
    print(f"  soil_water_content: "
          f"min={theta.min():.4f}  mean={theta.mean():.4f}  max={theta.max():.4f}")
    ds.close()


if __name__ == "__main__":
    p = argparse.ArgumentParser(
        description="VAMPS BMI gridded runner — streams forcing/output per timestep"
    )
    p.add_argument("forcing", help="input forcing NetCDF (time, ny, nx)")
    p.add_argument("--inp",   required=True,
                   help="VAMPS .inp configuration file")
    p.add_argument("out",     nargs="?", default="out.nc",
                   help="output NetCDF file (default: out.nc)")
    a = p.parse_args()
    run(forcing_path=a.forcing, inp_file=a.inp, out_path=a.out)
