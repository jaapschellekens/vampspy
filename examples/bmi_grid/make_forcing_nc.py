"""
make_forcing_nc.py — Convert Fiji .prn forcing files to a gridded NetCDF.

Creates a (time, ny, nx) forcing file by tiling the 1-D Fiji time series
across a spatial grid and adding small random perturbations to precipitation
and air temperature.

Usage
-----
    python make_forcing_nc.py [--ny NY] [--nx NX] [out.nc]
"""
from __future__ import annotations

import argparse
import os
import sys

import numpy as np

try:
    import netCDF4 as nc
except ImportError:
    sys.exit("netCDF4 is required:  pip install netCDF4")

from vampspy.forcing import read_ts, read_ts_timed

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
FIJI_DIR   = os.path.join(SCRIPT_DIR, "..", "fiji")

_FORCING_FILES = {
    "precipitation":      "precip.prn",
    "net_radiation":      "rnet.prn",
    "incoming_radiation": "inrad.prn",
    "relative_humidity":  "rh.prn",
    "air_temperature":    "newt.prn",
    "wind_speed":         "wind.prn",
}

_UNITS = {
    "precipitation":      "cm d-1",
    "net_radiation":      "W m-2",
    "incoming_radiation": "W m-2",
    "relative_humidity":  "1",
    "air_temperature":    "degC",
    "wind_speed":         "m s-1",
}


def main(ny: int = 4, nx: int = 5, steps: int | None = None,
         out_path: str = "forcing.nc") -> None:
    # Load base time series (use precip for the shared time axis)
    time_vals, _ = read_ts_timed(os.path.join(FIJI_DIR, "precip.prn"))
    if steps is not None:
        time_vals = time_vals[:steps]
    nsteps = len(time_vals)

    # Spatial perturbation fields (fixed in time)
    rng        = np.random.default_rng(42)
    pre_factor = rng.uniform(0.8, 1.2, (ny, nx))
    tem_offset = rng.uniform(-1.0, 1.0, (ny, nx))

    ds = nc.Dataset(out_path, "w", format="NETCDF4")
    ds.Conventions = "CF-1.8"
    ds.title       = "VAMPS gridded forcing (derived from Fiji example)"
    ds.source      = "make_forcing_nc.py"

    ds.createDimension("time", nsteps)
    ds.createDimension("ny",   ny)
    ds.createDimension("nx",   nx)

    vt = ds.createVariable("time", "f8", ("time",))
    vt.units     = "d"
    vt.long_name = "model time"
    vt[:]        = time_vals

    for bmi_name, fname in _FORCING_FILES.items():
        path = os.path.join(FIJI_DIR, fname)
        base = read_ts(path)
        if len(base) < nsteps:
            base = np.pad(base, (0, nsteps - len(base)), constant_values=base[-1])
        base = base[:nsteps]                       # (nsteps,)
        grid = np.broadcast_to(base[:, None, None], (nsteps, ny, nx)).copy()

        if bmi_name == "precipitation":
            grid *= pre_factor[None, :, :]
        elif bmi_name == "air_temperature":
            grid += tem_offset[None, :, :]

        v = ds.createVariable(bmi_name, "f4", ("time", "ny", "nx"),
                              zlib=True, complevel=4)
        v.units = _UNITS[bmi_name]
        v[:]    = grid.astype("f4")

    ds.close()
    print(f"Written: {out_path}  ({nsteps} steps, {ny}×{nx} grid)")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description="Build a gridded forcing NetCDF from Fiji .prn files")
    p.add_argument("--ny",    type=int, default=4)
    p.add_argument("--nx",    type=int, default=5)
    p.add_argument("--steps", type=int, default=None,
                   help="number of timesteps to write (default: all rows in forcing files)")
    p.add_argument("out",     nargs="?", default="forcing.nc")
    a = p.parse_args()
    main(ny=a.ny, nx=a.nx, steps=a.steps, out_path=a.out)
