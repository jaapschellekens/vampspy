"""
soilf_bc.py — Python implementation of the Brooks and Corey (1964)
soil water retention and hydraulic conductivity functions.

Use as a method=5 (Python user-defined) soil model.  Import from your
xtrapy script to activate it::

    from soilf_bc import *

Parameters are read from the soil section of the .inp file:
  lambda          — pore-size distribution index
  hb              — air-entry (bubbling) pressure head (cm, negative)
  thetas          — saturated water content [-]
  ksat            — saturated hydraulic conductivity [cm/day]
  theta_residual  — residual water content [-]

Governing equations (h is the pressure head, negative for unsaturated):
  Se(h)  = (hb/h)^lambda          for h < hb,  1 otherwise
  theta  = theta_r + (theta_s - theta_r) * Se
  K(Se)  = Ksat * Se^(2/lambda + 3)
  K(h)   = Ksat * (hb/h)^n        n = 2 + 3*lambda
"""
import vamps

# Module-level parameters; populated by getspars() for each soil type nr.
_lambda = {}   # pore-size distribution index
_hb     = {}   # air-entry pressure head [cm, negative]
_n      = {}   # conductivity exponent = 2 + 3*lambda


def getspars(section, nr):
    """Read Brooks-Corey parameters from the .inp file for soil type nr."""
    lam        = float(vamps.getdefstr(section, "lambda", "0.5"))
    _lambda[nr] = lam
    _hb[nr]    = float(vamps.getdefstr(section, "hb", "-10.0"))
    _n[nr]     = 2.0 + 3.0 * lam


def h2dmc(nr, head):
    """Differential moisture capacity as a function of pressure head."""
    hb = _hb[nr]
    if head >= hb:
        return 0.0
    thetas   = vamps.getspar(nr, "thetas")
    residual = vamps.getspar(nr, "residual_water")
    lam = _lambda[nr]
    # dtheta/dh = (thetas - thetar) * lambda * (hb/h)^lambda / (-h)
    Se = (hb / head) ** lam
    return (thetas - residual) * lam * Se / (-head)


def t2k(nr, wcon):
    """Hydraulic conductivity from water content."""
    thetas   = vamps.getspar(nr, "thetas")
    residual = vamps.getspar(nr, "residual_water")
    ksat     = vamps.getspar(nr, "ksat")
    lam = _lambda[nr]

    relsat = (wcon - residual) / (thetas - residual)
    if relsat > 1.0:
        relsat = 1.0
    if relsat < 0.001:
        return 1.0e-10
    # K = Ksat * Se^(2/lambda + 3)
    return ksat * relsat ** (2.0 / lam + 3.0)


def t2h(nr, wcon, depth):
    """Pressure head from water content."""
    thetas   = vamps.getspar(nr, "thetas")
    residual = vamps.getspar(nr, "residual_water")
    hb  = _hb[nr]
    lam = _lambda[nr]

    if (thetas - wcon) < 1.0e-6:
        return abs(depth)
    if (wcon - residual) < 1.0e-6:
        return -1.0e16
    Se = (wcon - residual) / (thetas - residual)
    # h = hb * Se^(-1/lambda)
    return hb * Se ** (-1.0 / lam)


def h2t(nr, head):
    """Water content from pressure head."""
    thetas   = vamps.getspar(nr, "thetas")
    residual = vamps.getspar(nr, "residual_water")
    hb  = _hb[nr]
    lam = _lambda[nr]

    if head >= hb:
        return thetas
    return residual + (thetas - residual) * (hb / head) ** lam


def h2k(nr, head):
    """Hydraulic conductivity from pressure head."""
    hb = _hb[nr]
    n  = _n[nr]
    ksat = vamps.getspar(nr, "ksat")

    if head >= hb:
        return ksat
    return ksat * (hb / head) ** n


def h2dkdp(nr, head):
    """d(K)/d(head) — analytical derivative."""
    hb = _hb[nr]
    n  = _n[nr]

    if head >= hb:
        return 0.0
    # dK/dh = n * K(h) / (-h)  (positive)
    return h2k(nr, head) * n / (-head)


def h2u(nr, head):
    """Matric flux potential (integral of K from -inf to head).

    Analytical solution:
      u(h) = -(K(h)*h)/(n-1)          for h < hb
      u(h) = Ksat*(-hb/(n-1) + h - hb)  for h >= hb
    """
    hb   = _hb[nr]
    n    = _n[nr]
    ksat = vamps.getspar(nr, "ksat")

    if head >= hb:
        return ksat * (-hb / (n - 1.0) + head - hb)
    else:
        return -(h2k(nr, head) * head) / (n - 1.0)
