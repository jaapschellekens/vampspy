"""
plot_bmi_output.py — Visualise VAMPS BMI gridded output.

Reads the NetCDF produced by run_bmi_grid.py and writes four PNGs:

  01_scalar_timeseries.png   — time series of all scalar outputs,
                               spatial mean ± min/max shading.
  02_spatial_final.png       — spatial heatmap (ny × nx) of selected
                               scalars at the last timestep.
  03_theta_profile.png       — mean soil-water-content profile (theta vs
                               layer index) at first, middle, and last step.
  04_spatial_variability.png — coefficient of variation of soil_water_volume
                               over time (ny × nx heatmap).

Usage
-----
    python plot_bmi_output.py out.nc [--outdir plots]
"""
from __future__ import annotations

import argparse
import os

import numpy as np

try:
    import netCDF4 as nc
except ImportError:
    raise SystemExit("netCDF4 is required:  pip install netCDF4")

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.gridspec as gridspec
except ImportError:
    raise SystemExit("matplotlib is required:  pip install matplotlib")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _load(path: str):
    ds = nc.Dataset(path, "r")
    data = {k: np.asarray(ds[k][:]) for k in ds.variables}
    ds.close()
    return data


def _savefig(fig, path: str) -> None:
    fig.savefig(path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  saved  {path}")


# ---------------------------------------------------------------------------
# Plot 1 — scalar time series
# ---------------------------------------------------------------------------

_SCALAR_LABELS = {
    "soil_water_volume":     ("Soil water volume",      "cm"),
    "soil_moisture_deficit": ("Soil moisture deficit",  "cm"),
    "surface_runoff":        ("Surface runoff",         "cm d⁻¹"),
    "bottom_drainage":       ("Bottom drainage",        "cm d⁻¹"),
    "avg_soil_water_content":("Avg soil water content", "—"),
    "transpiration_flux":    ("Transpiration",          "cm d⁻¹"),
    "soil_evaporation_flux": ("Soil evaporation",       "cm d⁻¹"),
    "interception_flux":     ("Interception",           "cm d⁻¹"),
}


def plot_scalar_timeseries(data: dict, outdir: str) -> None:
    t     = data["time"]
    names = [n for n in _SCALAR_LABELS if n in data]
    ncols = 2
    nrows = (len(names) + 1) // ncols
    fig, axes = plt.subplots(nrows, ncols, figsize=(13, 3 * nrows),
                             sharex=True)
    axes = np.array(axes).ravel()

    for ax, name in zip(axes, names):
        arr  = data[name]                        # (time, ny, nx)
        flat = arr.reshape(len(t), -1)           # (time, ncols)
        mn   = flat.min(axis=1)
        mx   = flat.max(axis=1)
        mean = flat.mean(axis=1)
        label, unit = _SCALAR_LABELS[name]
        ax.fill_between(t, mn, mx, alpha=0.25, label="min–max")
        ax.plot(t, mean, lw=1.5,  label="spatial mean")
        ax.set_title(label, fontsize=9)
        ax.set_ylabel(unit,  fontsize=8)
        ax.legend(fontsize=7, loc="upper right")
        ax.tick_params(labelsize=7)

    # Hide unused axes
    for ax in axes[len(names):]:
        ax.set_visible(False)

    for ax in axes[-ncols:]:
        ax.set_xlabel("model time (d)", fontsize=8)

    fig.suptitle("Scalar outputs — spatial mean ± min/max", fontsize=11)
    fig.tight_layout()
    _savefig(fig, os.path.join(outdir, "01_scalar_timeseries.png"))


# ---------------------------------------------------------------------------
# Plot 2 — spatial heatmaps at final timestep
# ---------------------------------------------------------------------------

_SPATIAL_VARS = [
    ("soil_water_volume",     "Soil water volume (cm)"),
    ("soil_moisture_deficit", "Soil moisture deficit (cm)"),
    ("avg_soil_water_content","Avg θ (—)"),
    ("transpiration_flux",    "Transpiration (cm d⁻¹)"),
]


def plot_spatial_final(data: dict, outdir: str) -> None:
    t    = data["time"]
    tidx = -1                                    # last timestep
    tval = float(t[tidx])

    names  = [n for n, _ in _SPATIAL_VARS if n in data]
    labels = {n: lbl for n, lbl in _SPATIAL_VARS}
    ncols  = 2
    nrows  = (len(names) + 1) // ncols
    fig, axes = plt.subplots(nrows, ncols, figsize=(10, 4 * nrows))
    axes = np.array(axes).ravel()

    for ax, name in zip(axes, names):
        grid = data[name][tidx]                  # (ny, nx)
        im   = ax.imshow(grid, origin="upper", aspect="auto",
                         interpolation="nearest")
        plt.colorbar(im, ax=ax, shrink=0.8)
        ax.set_title(labels[name], fontsize=9)
        ax.set_xlabel("x", fontsize=8)
        ax.set_ylabel("y", fontsize=8)
        ax.tick_params(labelsize=7)

    for ax in axes[len(names):]:
        ax.set_visible(False)

    fig.suptitle(f"Spatial maps at final timestep  t = {tval:.0f}", fontsize=11)
    fig.tight_layout()
    _savefig(fig, os.path.join(outdir, "02_spatial_final.png"))


# ---------------------------------------------------------------------------
# Plot 3 — theta profile at first / middle / last timestep
# ---------------------------------------------------------------------------

def plot_theta_profile(data: dict, outdir: str) -> None:
    if "soil_water_content" not in data:
        print("  skip  03_theta_profile.png  (soil_water_content not found)")
        return

    theta = data["soil_water_content"]           # (time, ny, nx, layer)
    t     = data["time"]
    ntime, ny, nx, nlayers = theta.shape

    t_indices = [0, ntime // 2, ntime - 1]
    t_labels  = [f"t={float(t[i]):.0f}" for i in t_indices]
    layers    = np.arange(nlayers)

    fig, ax = plt.subplots(figsize=(5, 7))
    colors  = ["#1f77b4", "#ff7f0e", "#2ca02c"]

    for idx, (ti, label, color) in enumerate(zip(t_indices, t_labels, colors)):
        profile = theta[ti].reshape(-1, nlayers)   # (ncols, layer)
        mean    = profile.mean(axis=0)
        mn      = profile.min(axis=0)
        mx      = profile.max(axis=0)
        ax.fill_betweenx(layers, mn, mx, alpha=0.2, color=color)
        ax.plot(mean, layers, lw=2, color=color, label=label)

    ax.invert_yaxis()
    ax.set_xlabel("Volumetric water content (—)", fontsize=9)
    ax.set_ylabel("Layer index (0 = top)",        fontsize=9)
    ax.set_title("Soil water content profile\n(spatial mean ± min/max)", fontsize=10)
    ax.legend(fontsize=8)
    ax.tick_params(labelsize=8)
    fig.tight_layout()
    _savefig(fig, os.path.join(outdir, "03_theta_profile.png"))


# ---------------------------------------------------------------------------
# Plot 4 — spatial coefficient of variation of soil_water_volume over time
# ---------------------------------------------------------------------------

def plot_spatial_variability(data: dict, outdir: str) -> None:
    if "soil_water_volume" not in data:
        print("  skip  04_spatial_variability.png")
        return

    v    = data["soil_water_volume"]             # (time, ny, nx)
    t    = data["time"]

    # temporal mean and std per grid cell
    tmean = v.mean(axis=0)                       # (ny, nx)
    tstd  = v.std(axis=0)
    with np.errstate(invalid="ignore", divide="ignore"):
        cv = np.where(tmean > 0, tstd / tmean, np.nan)

    fig, axes = plt.subplots(1, 2, figsize=(10, 4))

    im0 = axes[0].imshow(tmean, origin="upper", aspect="auto",
                          interpolation="nearest")
    plt.colorbar(im0, ax=axes[0], shrink=0.85)
    axes[0].set_title("Time-mean soil water volume (cm)", fontsize=9)

    im1 = axes[1].imshow(cv, origin="upper", aspect="auto",
                          interpolation="nearest", cmap="YlOrRd")
    plt.colorbar(im1, ax=axes[1], shrink=0.85)
    axes[1].set_title("Coefficient of variation (—)", fontsize=9)

    for ax in axes:
        ax.set_xlabel("x", fontsize=8)
        ax.set_ylabel("y", fontsize=8)
        ax.tick_params(labelsize=7)

    fig.suptitle("Spatial variability of soil water volume over simulation", fontsize=10)
    fig.tight_layout()
    _savefig(fig, os.path.join(outdir, "04_spatial_variability.png"))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    p = argparse.ArgumentParser(
        description="Plot VAMPS BMI gridded output NetCDF to PNG files"
    )
    p.add_argument("nc_file",  help="output NetCDF from run_bmi_grid.py")
    p.add_argument("--outdir", default="plots",
                   help="directory for PNG output (default: plots/)")
    a = p.parse_args()

    os.makedirs(a.outdir, exist_ok=True)
    print(f"Reading  {a.nc_file}")
    data = _load(a.nc_file)

    ntime = len(data["time"])
    ny    = data["soil_water_volume"].shape[1]
    nx    = data["soil_water_volume"].shape[2]
    print(f"  {ntime} timesteps  {ny}×{nx} grid  → writing to {a.outdir}/")

    plot_scalar_timeseries(data, a.outdir)
    plot_spatial_final(data, a.outdir)
    plot_theta_profile(data, a.outdir)
    plot_spatial_variability(data, a.outdir)

    print("Done.")


if __name__ == "__main__":
    main()
