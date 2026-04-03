"""
setup.py — build the vampspy._vampscore C extension.

The extension compiles the VAMPS solver sources directly (no pre-built
libraries needed).  Run:

    pip install -e .[notebook]

Prerequisites:
    ./configure   must have been run in src/ (generates src/vconfig.h)
"""

import os
import subprocess
from setuptools import setup, Extension

try:
    import numpy as np
    numpy_inc = np.get_include()
except ImportError:
    raise RuntimeError("numpy must be installed before building vampspy")

ROOT = os.path.dirname(os.path.abspath(__file__))
SRC  = os.path.join(ROOT, "src")

def _src(*parts):
    """Return a path relative to setup.py (required by setuptools ≥ 72)."""
    return os.path.relpath(os.path.join(ROOT, *parts))

# ---------------------------------------------------------------------------
# Source files — everything the solver needs, minus main() (guarded by
# VAMPS_EXT_BUILD) and minus init_py.c (Python embedding not needed here).
# ---------------------------------------------------------------------------
UTIL  = [_src("src", "util", f) for f in [
    "dataset.c", "mktable.c", "output.c", "progress.c", "memory.c",
    "resamp.c", "perror.c", "pro_mesg.c",
    "utout.c",   # print* helpers (printfl, printint, printar …) used by
                 # canopy.c, topout.c, mktable.c, pro_mesg.c
    # removed: getopt.c (CLI only), xtraout.c (dorun() only — xopenout/xtraout
    #          are guarded with #ifndef VAMPS_EXT_BUILD in vamps.c),
    #          smoothar.c (no call sites in codebase)
]]
MAIN  = [_src("src", "main", f) for f in [
    "vamps.c", "vamps_ext.c", "soil_api.c",
    # removed: plot.c (gnuplot, CLI only), sigs.c (signal handling, CLI only)
]]
TOPSYS = [_src("src", "topsys", f) for f in [
    "intopsys.c", "notree.c", "topout.c", "pre_can.c", "canopy.c",
]]
SOIL  = [_src("src", "soil", f) for f in [
    "swatsoil.c", "soilut.c", "array.c", "getparm.c", "alloc.c",
    "smooth.c", "misc_p.c", "filltab.c", "reduceva.c", "det_hatm.c",
    "rootex.c", "setzero.c", "fluxes.c", "calcgwl.c", "band.c",
    "soilboun.c", "headcalc.c", "timestep.c", "integral.c", "soilout.c",
]]
TS_LIB = [_src("src", "ts.lib", f) for f in [
    "ts_com.c", "ts_input.c", "ts_mem.c", "ts_readf.c", "ts_spl.c",
    "ts_time.c",
]]
DEF_LIB = [_src("src", "deffile.lib", f) for f in [
    "deffile.c", "fgets.c", "index.c", "memlist.c", "strcmp.c",
]]
MET_LIB = [_src("src", "met.lib", f) for f in [
    "e0.c", "eaes.c", "earo.c", "gamma.c", "int.c", "lambda.c",
    "makkink.c", "penmon.c", "ra.c", "rn_open.c", "vslope.c",
]]
MAQ_LIB = []  # marquard.c only used for method=3 which raises Perror; removed from ext build
NRU_LIB = [_src("src", "nr_ut.lib", f) for f in [
    "log.c", "nr_free.c", "nr_mat.c", "nr_rw.c", "nr_stat.c",
    "nr_ut.c", "nr_vec.c", "nrutil.c",
]]
EXT_SRC = [_src("vampspy", "_vampscore.c")]

sources = (EXT_SRC + MAIN + UTIL + TOPSYS + SOIL +
           TS_LIB + DEF_LIB + MET_LIB + MAQ_LIB + NRU_LIB)

# ---------------------------------------------------------------------------
# Compiler/linker flags
# ---------------------------------------------------------------------------
inc_dirs = [
    numpy_inc,
    _src("src", "include"),
    _src("src"),
]

macros = [
    ("HAVE_CONFIG_H", None),   # include src/vconfig.h
    ("VAMPS_EXT_BUILD", None), # suppress main() and Python-embedding code
    ("VAMPSLIB", '""'),        # compiled-in share path (not used in ext mode)
    ("GNUPLOT",  '""'),
]

ext = Extension(
    "vampspy._vampscore",
    sources=sources,
    include_dirs=inc_dirs,
    define_macros=macros,
    extra_compile_args=["-g", "-O2", "-ffast-math", "-march=native", "-w"],  # -w silences old-code warnings
    extra_link_args=["-lm"],
)

setup(ext_modules=[ext])
