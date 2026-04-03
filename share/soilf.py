"""
soilf.py — Python implementation of the Clapp/Hornberger (Campbell 1974)
soil theta/h/k relationship functions.

Port of soilf.sl.  Provides user-defined soil functions for VAMPS soil
method = 5.  Import this module from your xtrapy script to activate it::

    from soilf import *

The five functions (getspars, h2dmc, t2k, t2h, h2t) are called by the C
engine via the function-pointer mechanism.  h2k and h2dkdp fall back to
the C Clapp implementation if not defined here (method=0 handles them).

Parameters are read from the soil section of the .inp file:
  b       — Clapp/Hornberger b exponent
  psisat  — air-entry suction head (cm, negative)
  thetas  — porosity (saturated water content)
  ksat    — saturated hydraulic conductivity (cm/day)
"""
import vamps

MAXSUCKHEAD = -1.0e20

# Module-level parameters; set by getspars() for each soil type
_b      = {}   # b[spnr]
_n      = {}   # n[spnr] = 2 + 3/b
_psisat = {}   # psisat[spnr]


def getspars(section, nr):
    """Read Clapp/Hornberger parameters from the .inp file for soil type nr."""
    b = float(vamps.getdefstr(section, "b", "0.0"))
    _b[nr]      = b
    _n[nr]      = 2.0 + 3.0 / b if b != 0.0 else 0.0
    _psisat[nr] = float(vamps.getdefstr(section, "psisat", "0.0"))


def h2dmc(nr, head):
    """Differential moisture capacity as a function of pressure head."""
    if head >= -0.1:
        return 0.0
    thetas = vamps.getspar(nr, "thetas")
    b      = _b[nr]
    psisat = _psisat[nr]
    tt = thetas / (-b * psisat)
    t  = (head / psisat) ** ((-1.0 / b) - 1.0)
    return t * tt


def t2k(nr, wcon):
    """Unsaturated hydraulic conductivity from water content."""
    thetas  = vamps.getspar(nr, "thetas")
    residual = vamps.getspar(nr, "residual_water")
    ksat    = vamps.getspar(nr, "ksat")
    b       = _b[nr]
    n       = _n[nr]

    relsat = (wcon - residual) / (thetas - residual)
    if relsat > 1.0:
        relsat = 1.0
    if relsat < 0.001:
        return 1.0e-10
    return ksat * (wcon / thetas) ** (b * n)


def t2h(nr, wcon, depth):
    """Pressure head from water content."""
    thetas  = vamps.getspar(nr, "thetas")
    residual = vamps.getspar(nr, "residual_water")
    b       = _b[nr]
    psisat  = _psisat[nr]

    if (thetas - wcon) < 1.0e-5:
        return abs(depth)
    if (wcon - residual) < 1.0e-5:
        return MAXSUCKHEAD * (1.0 - (wcon - residual))
    ans = psisat * (wcon / thetas) ** (-b)
    if ans < MAXSUCKHEAD:
        return MAXSUCKHEAD * (1.0 - (wcon - residual))
    return ans


def h2t(nr, head):
    """Water content from pressure head."""
    thetas  = vamps.getspar(nr, "thetas")
    psisat  = _psisat[nr]
    b       = _b[nr]

    if head >= -psisat:
        return thetas
    return thetas * (head / psisat) ** (-1.0 / b)


def h2k(nr, head):
    """Hydraulic conductivity from pressure head."""
    wcon = h2t(nr, head)
    return t2k(nr, wcon)


def h2dkdp(nr, head):
    """d(k)/d(head) — approximate numerical derivative."""
    dh = 1.0e-4 * abs(head) if head != 0.0 else 1.0e-6
    return (h2k(nr, head + dh) - h2k(nr, head - dh)) / (2.0 * dh)


def h2u(nr, head):
    """Matric flux potential (integral of k from -inf to head).

    Returns 0 (placeholder) — VAMPS only uses this with lookup tables.
    """
    return 0.0
