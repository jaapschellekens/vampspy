"""
topsys.py — Python scripted topsystem for VAMPS (top-system = 6).

Port of topsys.sl.  Implements a scripted topsystem in which soil
evaporation, potential transpiration, interception, and precipitation
are pre-calculated from time-series files.

This gives the same results as topsystem 4 (TOP_PRE_CANOP) but lets
the user override fluxes via Python.

To activate, add to your .inp file::

    [top]
    system = 6

and import this module from your xtrapy script::

    from topsys import pre_top, post_top, tstep_top

Time-series keys read from the [ts] section:
  spe  — potential soil evaporation [cm]
  ptr  — potential transpiration [cm]
  inr  — interception [cm]
  pre  — precipitation [cm]  (always present as vamps.id_pre)
"""
import vamps

_spe_id = None
_ptr_id = None
_inr_id = None


def pre_top():
    """Initialise the topsystem: load time-series and record dataset IDs."""
    global _spe_id, _ptr_id, _inr_id
    print("-" * 55)
    print("topsys.py — Python scripted topsystem")
    print("Experimental: all fluxes pre-calculated from files.")
    print("Should give the same results as topsystem 4.")
    print("-" * 55)
    spe_fname = vamps.getdefstr("ts", "spe", "spe.prn")
    inr_fname = vamps.getdefstr("ts", "inr", "inr.prn")
    ptr_fname = vamps.getdefstr("ts", "ptr", "ptr.prn")
    _spe_id = vamps.readset(spe_fname, "spe")
    _inr_id = vamps.readset(inr_fname, "inr")
    _ptr_id = vamps.readset(ptr_fname, "ptr")


def post_top():
    """Clean up after the run (currently a no-op)."""
    pass


def tstep_top(tstep):
    """Return fluxes for timestep *tstep*.

    Returns
    -------
    tuple
        (precipitation, interception, transpiration, soilevaporation)
    """
    prec  = vamps.getset_y(vamps.id_pre, tstep)
    intc  = vamps.getset_y(_inr_id,      tstep)
    trans = vamps.getset_y(_ptr_id,      tstep)
    sev   = vamps.getset_y(_spe_id,      tstep)
    return (prec, intc, trans, sev)
