"""
met.py — Meteorological functions for VAMPS Python scripting.

Port of met.sl.  Pure math — no dependency on vamps module state.

Functions
---------
eaes(td, rh)       -> (ea, es)   actual and saturation vapour pressure
vslope(td, es)     -> slope       slope of the vapour pressure curve
gamma(td)          -> gamma       psychrometric constant [mbar/K]
e0a(...)           -> e0          Penman open-water evaporation (method a)
e0b(...)           -> e0          Penman open-water evaporation (method b)
makkink(rad, slope, gamma) -> Emak  Makkink reference evaporation [mm/day]
ra(z, z0, d, u)    -> ra          aerodynamic resistance [s/m]
"""

import math


def eaes(td, rh):
    """Actual (ea) and saturation (es) vapour pressure [mbar].

    Parameters
    ----------
    td : float   dry bulb temperature [°C]
    rh : float   relative humidity [%]

    Returns
    -------
    (ea, es) both in mbar
    """
    tkel = td + 273.15
    log10 = math.log(10.0)
    es = math.exp(log10 * (
        10.79574 * (1 - 273.15 / tkel)
        - 5.028 * math.log(tkel / 273.15) / log10
        + 1.50475e-4 * (1 - math.exp(log10 * -8.2969 * (tkel / 273.15 - 1)))
        + 0.42873e-3 * math.exp(log10 * 4.76955 * (1 - 273.15 / tkel))
        + 0.78614
    ))
    ea = rh * es / 100.0
    return ea, es


def vslope(td, es):
    """Slope of the saturation vapour pressure curve [mbar/K].

    Parameters
    ----------
    td : float   dry bulb temperature [°C]
    es : float   saturation vapour pressure [mbar]
    """
    TEMP = td + 273.15
    log10 = math.log(10.0)
    slope = es * (
        10.79574 / 0.4343 * (273.15 / (TEMP * TEMP))
        - 5.028 / TEMP
        + 1.50475e-4 * 8.2969 / (0.4343 * 0.4343) / 273.15
          * math.exp(log10 * 8.2969 * (1 - TEMP / 273.15))
        + 0.42873e-3 * 4.76955 / (0.4343 * 0.4343) * 273.15 / (TEMP * TEMP)
          * math.exp(log10 * 4.76955 * (1 - 273.15 / TEMP))
    )
    return slope


def gamma(td):
    """Psychrometric constant [mbar/°C].

    Assumes air pressure = 998 mbar, Cp = 1005 J/(kg·K).
    """
    p  = 998.0
    cp = 1005.0
    l  = 2.501 - 0.00237 * td          # latent heat of vaporisation [MJ/kg]
    return cp * p / (0.622 * l)


def e0a(Td, Rs, sunratio, u, Ea, Es, Slope, Gamma, L):
    """Penman open-water evaporation [mm/day] — method a (Calder 1990).

    Parameters
    ----------
    Td       : float   dry bulb temperature [°C]
    Rs       : float   mean daily incoming shortwave radiation [W/m²]
    sunratio : float   sunshine ratio (n/N) [–]
    u        : float   mean daily wind speed at 2 m [m/s]
    Ea       : float   actual vapour pressure [mbar]
    Es       : float   saturation vapour pressure [mbar]
    Slope    : float   slope of the vap. pressure curve [mbar/°C]
    Gamma    : float   psychrometric constant [mbar/°C]
    L        : float   latent heat of vaporisation [J/kg]
    """
    Ea    = Ea    * 0.1     # mbar → kPa
    Es    = Es    * 0.1
    Slope = Slope * 0.1
    Gamma = Gamma * 0.1

    r     = 0.06            # open-water albedo
    sigma = 5.67e-8         # Stefan-Boltzmann [W/m²/K⁴]
    T4    = (Td + 273.15) ** 4
    Rl    = (86400 * sigma * T4
             * (0.56 - 0.248 * math.sqrt(Ea))
             * (0.1 + 0.9 * sunratio) / L)
    Rnet  = Rs * 86400 * (1 - r) / L - Rl
    Eaero = 2.6 * (Es - Ea) * (1 + 0.537 * u)

    return (Slope * Rnet + Gamma * Eaero) / (Slope + Gamma)


def e0b(Td, Rs, Rsout, Rnet, u, Ea, Es, Slope, Gamma, L):
    """Penman open-water evaporation [mm/day] — method b (net radiation).

    Parameters
    ----------
    Td     : float   dry bulb temperature [°C]
    Rs     : float   incoming shortwave radiation [W/m²]
    Rsout  : float   outgoing shortwave radiation [W/m²]
    Rnet   : float   measured net radiation [W/m²]
    u      : float   wind speed [m/s]
    Ea     : float   actual vapour pressure [mbar]
    Es     : float   saturation vapour pressure [mbar]
    Slope  : float   slope of the vap. pressure curve [mbar/°C]
    Gamma  : float   psychrometric constant [mbar/°C]
    L      : float   latent heat of vaporisation [J/kg]
    """
    Ea    = Ea    * 0.10
    Es    = Ea    * 0.10    # note: intentional copy from original
    Slope = Slope * 0.10
    Gamma = Gamma * 0.10

    r        = 0.06
    Rl       = Rs - Rnet - Rsout
    Rnetopen = (Rs * (1 - r) - Rl) * 86400 / L
    Eaero    = 2.6 * (Es - Ea) * (1 + 0.537 * u)

    return (Slope * Rnetopen + Gamma * Eaero) / (Slope + Gamma)


def makkink(rad, Slope, Gamma):
    """Makkink reference evaporation [mm/day].

    C1 = 0.65, C2 = 0.0 (standard constants).

    Parameters
    ----------
    rad   : float   net radiation [W/m²]
    Slope : float   slope of the vap. pressure curve [mbar/°C]
    Gamma : float   psychrometric constant [mbar/°C]
    """
    C1 = 0.65
    C2 = 0.0
    return 1000 * C1 * Slope / (Slope + Gamma * 0.1) * rad + C2


def ra(z, z0, d, u):
    """Aerodynamic resistance [s/m].

    ra = [ln((z-d)/z0)]² / (k²·u)   where k = 0.41 (von Kármán constant).

    Parameters
    ----------
    z  : float   measurement height [m]
    z0 : float   roughness length [m]
    d  : float   displacement height [m]
    u  : float   wind speed [m/s]
    """
    k2 = 0.1681   # k² = 0.41²
    tt = math.log((z - d) / z0)
    return (tt * tt / k2) / u
