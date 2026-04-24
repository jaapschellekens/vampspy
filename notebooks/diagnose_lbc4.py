"""
Diagnostic: compare initprof=0 vs initprof=2 with bottom=4 (fixed head BC).

Test A: initprof=0 — theta_initial given, h computed from theta (h≈0 for theta_s),
        then bottom node overridden to h=+53cm → large hydraulic gradient.
Test B: initprof=2 — hydrostatic initialization from gw_initial, h consistent
        with fixed head → small hydraulic gradient at bottom.

Both tests run only 3 timesteps (steps=3) and measure wall-clock time.
"""
import os
import sys
import time
import tempfile
import shutil

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

FIJI_DIR = os.path.abspath('../examples/fiji')

BASE_CONFIG = """\
[vamps]
iniinmem=1

[run]
outputfile = {outfile}

[determine]
soilmoisture = 1

[top]
system = 4

[time]
steps = 30

[ts]
pre = precip_tiled.prn
ptr = ptr.prn
spe = spe.prn
inr = inr.prn
hea = hea.prn

[roots]
swsink = 1
swhypr = 0
swupfu = 0
depth  = 120.0
hlim1  = -5.0
hlim2u = -50.0
hlim2l = -50.0
hlim3h = -800.0
hlim3l = -1000.0
hlim3  = -1800.0
hlim4  = -12000.0

[soil]
dtmin   = 0.1E-3
dtmax   = 0.5E-2
mbck    = 0
swredu  = 1
smooth  = 5
cofred  = 0.35
outdir  = output
pondmx  = 0.0
verbose = 1
layers  = 77
bottom  = 4
initprof = {initprof}
mktable  = 1
dumptables = 1
{extra_soil}
theta_initial = 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.640000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.600000 0.597601 0.593870 0.590430 0.587240 0.584268 0.581486 0.578874 0.576411 0.574083 0.571876 0.569778 0.567780 0.565873 0.564050 0.562303

[layer_0]
description  = Tulasewa top layer
thickness    = 2.000000
soilsection  = st_0

[st_0]
method         = 6
thetas         = 0.6
theta_residual = 0.08
lambda         = 0.098
hb             = -16.4
ksat           = 1800

[layer_14]
description  = Tulasewa 30-75 cm layer
thickness    = 2.000000
soilsection  = st_1

[st_1]
method         = 6
thetas         = 0.64
theta_residual = 0.08
lambda         = 0.094
hb             = -23.8
ksat           = 380.0

[layer_36]
description  = Tulsewa deep layer > 75 cm
thickness    = 2.000000
soilsection  = st_2

[st_2]
method         = 6
thetas         = 0.6
theta_residual = 0.08
lambda         = 0.094
hb             = -23.8
ksat           = 3.0
"""


def run_test(label, initprof, extra_soil='', timeout=60):
    outfile = f'diag_{label}.out'
    cfg = BASE_CONFIG.format(
        outfile=outfile,
        initprof=initprof,
        extra_soil=extra_soil,
    )
    cfg_path = os.path.join(FIJI_DIR, f'diag_{label}.inp')
    with open(cfg_path, 'w') as f:
        f.write(cfg)

    orig_dir = os.getcwd()
    os.chdir(FIJI_DIR)
    try:
        from vampspy.model import Model
        try:
            m = Model.from_file(f'diag_{label}.inp')
        except Exception as e:
            print(f"  [{label}] LOAD ERROR: {e}")
            return None, None
        t0 = time.perf_counter()
        try:
            result = m.run()
        except Exception as e:
            elapsed = time.perf_counter() - t0
            print(f"  [{label}] RUN EXCEPTION after {elapsed:.1f}s: {e}")
            return elapsed, None
        elapsed = time.perf_counter() - t0
        return elapsed, result
    finally:
        os.chdir(orig_dir)


def analyse_h_gradient():
    """Compute the bottom-node hydraulic gradient for initprof=0 vs initprof=2."""
    import numpy as np

    # Soil parameters for bottom section (st_2)
    thetas = 0.6
    theta_r = 0.08
    lam = 0.094
    hb = -23.8   # cm (bubbling pressure, negative)
    ksat = 3.0   # cm/d
    dz = 2.0     # cm (layer thickness)

    # theta for layers 75 and 76 from theta_initial
    theta_initial = [0.565873, 0.564050, 0.562303]  # layers 74,75,76

    def t2h(theta):
        # Brooks-Corey: theta = (thetas-theta_r)*(hb/h)^lambda + theta_r  for h<hb
        # Invert: (hb/h)^lambda = (theta - theta_r)/(thetas - theta_r)
        # h = hb / ((theta - theta_r)/(thetas - theta_r))^(1/lambda)
        se = (theta - theta_r) / (thetas - theta_r)
        se = min(se, 1.0)
        if se >= 1.0:
            return 0.0  # saturated → h = 0
        return hb / (se ** (1.0 / lam))

    h_from_theta = [t2h(th) for th in theta_initial]
    gw_initial_cm = 100.0  # cm = 1000 mm

    # initprof=0: h computed from theta, last node overridden to hea=53 cm
    h_A = list(h_from_theta)
    h_A[2] = 53.0  # layer 76 overridden to hea

    # initprof=2: hydrostatic from gw_initial
    # In VAMPS: gwl[0] = -fabs(gw_initial), z[i] = negative depth below surface
    # h[i] = gwl[0] - z[i]  e.g. gwl=-100, z[75]=-151 → h=+51cm
    gwl = -gw_initial_cm
    z = [-149.0, -151.0, -153.0]  # negative depths (VAMPS convention)
    h_B = [gwl - zi for zi in z]  # = gw_initial - |z|

    print("=== Hydraulic gradient analysis at bottom boundary ===\n")
    print(f"{'':30s}  {'initprof=0 (Test A)':>22s}  {'initprof=2 (Test B)':>22s}")
    print("-" * 78)
    for i, (name, ha, hb_val) in enumerate(zip(
        ['layer 74', 'layer 75 (2nd-last)', 'layer 76 (bottom, hea)'],
        h_A, h_B
    )):
        print(f"  h[{i+74:2d}] ({name:18s}):  {ha:+10.2f} cm          {hb_val:+10.2f} cm")

    # Hydraulic gradient at bottom: (h[76]-h[75])/dz + 1 (positive = upward)
    grad_A = (h_A[2] - h_A[1]) / dz + 1.0
    grad_B = (h_B[2] - h_B[1]) / dz + 1.0

    # Darcy flux: q = k * gradient  (using ksat as upper bound)
    # In practice K(theta) for unsaturated node 75 in Test A could be much less
    q_A = ksat * grad_A
    q_B = ksat * grad_B

    print()
    print(f"  Hydraulic gradient (h[76]-h[75])/{dz:.0f}cm + 1:")
    print(f"    Test A: ({h_A[2]:+.1f} - {h_A[1]:+.1f})/{dz:.0f} + 1 = {grad_A:+.2f}")
    print(f"    Test B: ({h_B[2]:+.1f} - {h_B[1]:+.1f})/{dz:.0f} + 1 = {grad_B:+.2f}")
    print()
    print(f"  Max Darcy flux at ksat={ksat} cm/d:")
    print(f"    Test A: {ksat} × {grad_A:.2f} = {q_A:+.1f} cm/d  ← FAR exceeds ksat!")
    print(f"    Test B: {ksat} × {grad_B:.2f} = {q_B:+.1f} cm/d  ← physically reasonable")
    print()
    print("  Implication for solver:")
    print("    Test A: solver must reduce dt drastically to handle extreme gradient.")
    print("            With dtmin=0.001d: ~1000 sub-steps/day × 3 days = ~3000 iterations")
    print("    Test B: gradient ≈ 1 (unit gradient), solver converges in normal dt steps.")


def main():
    print("=" * 70)
    print("VAMPS bottom=4 convergence diagnostic")
    print("=" * 70)
    print()

    analyse_h_gradient()

    print()
    print("=" * 70)
    print("Running VAMPS model (30 timesteps each) ...")
    print("=" * 70)
    print()

    print("Test A: initprof=0 (theta_initial → h≈0, overridden bottom h=+53cm)")
    print("  Running (timeout ~60s) ...")
    elapsed_A, model_A = run_test('A_initprof0', initprof=0)
    if model_A is not None:
        print(f"  Completed in {elapsed_A:.3f}s")
    else:
        print(f"  Did not complete (after {elapsed_A:.1f}s)")

    print()
    print("Test B: initprof=2 (hydrostatic from gw_initial=100cm, consistent with hea=53cm)")
    print("  Running ...")
    elapsed_B, model_B = run_test('B_initprof2', initprof=2, extra_soil='gw_initial = 100.0')
    if model_B is not None:
        print(f"  Completed in {elapsed_B:.3f}s")
    else:
        print(f"  Did not complete (after {elapsed_B:.1f}s)")

    print()
    print("=" * 70)
    print("Summary")
    print("=" * 70)
    if model_A is not None and model_B is not None:
        ratio = elapsed_A / elapsed_B if elapsed_B > 0 else float('inf')
        print(f"  Test A (initprof=0): {elapsed_A:.3f}s")
        print(f"  Test B (initprof=2): {elapsed_B:.3f}s")
        print(f"  Slowdown factor: {ratio:.1f}×")
    else:
        print(f"  Test A: {'OK' if model_A else 'TIMEOUT/ERROR'} ({elapsed_A:.1f}s)")
        print(f"  Test B: {'OK' if model_B else 'TIMEOUT/ERROR'} ({elapsed_B:.1f}s)")


if __name__ == '__main__':
    main()
