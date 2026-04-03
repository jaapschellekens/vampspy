"""
regress.py — Regression equations for VAMPS Python scripting.

Port of regress.sl.  Import this module in your script to use the
functions, or define estrs() in your script to override the default.

Example — use the built-in Fiji pine-forest rs regression:

    from regress import estrs   # makes estrs() visible as a top-level hook
"""

import math
import vamps


# ---------------------------------------------------------------------------
# LAI regression helpers
# ---------------------------------------------------------------------------

def lai_to_s(lai):
    """Canopy storage capacity from leaf area index (lai*0.3)."""
    return lai * 0.3


def lai_to_pfree(lai):
    """Free throughfall coefficient from LAI."""
    return lai * 0.001


def lai_to_ptr(lai):
    """Fraction of rain diverted to trunk from LAI."""
    return lai * 0.001


# ---------------------------------------------------------------------------
# Radiation conversion
# ---------------------------------------------------------------------------

def Rs_to_Rn(Rs):
    """Convert incoming shortwave radiation to net radiation (Shuttleworth 1984)."""
    netrad = Rs * 0.858 - 35.0
    return max(netrad, 0.0)


# ---------------------------------------------------------------------------
# Surface resistance regression (Fiji pine forest example)
# ---------------------------------------------------------------------------

def estrs():
    """Estimate surface resistance from meteorological inputs.

    Uses the regression equations from a Fiji pine forest study.
    Requires time-series sets: nra (net radiation), pre (precipitation),
    win (wind speed), tem (air temperature).

    Returns rs in s/m.
    """
    step = vamps.thisstep

    A = vamps.getset_y(vamps.id_nra, step)   # net radiation  [W/m2]
    P = vamps.getset_y(vamps.id_pre, step)   # precipitation  [cm]
    U = vamps.getset_y(vamps.id_win, step)   # wind speed     [m/s]
    T = vamps.getset_y(vamps.id_tem, step)   # air temperature [°C]

    # Rain → stomata open
    if P > 0.0:
        return 0.0

    if A < 0.0:
        A = 0.0

    daypart = vamps.t - math.floor(vamps.t)

    daytime = 1
    if daypart > 0.708:   # after ~1700
        daytime = 0
    if daypart < 0.333:   # before ~0800
        daytime = 0

    vpd = vamps.VPD
    smd = vamps.SMD

    if daytime == 1 and A > 150.0:
        result = 28.89 - 0.0684 * A + 4.27 * vpd + 1.06 * smd
    else:
        result = 219.2 - 3.33 * A - 2.125 * vpd - 32.706 * U + 18.09 * T + 9.87 * smd

    # High radiation at night
    if A >= 150.0 and daytime == 0:
        result = 28.89 - 0.0684 * A + 4.27 * vpd + 1.06 * smd

    if result < 40.0 and A < 150.0:
        result = 28.89 - 0.0684 * A + 4.27 * vpd + 1.06 * smd

    return 1.39 * result
