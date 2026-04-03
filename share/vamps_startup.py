"""
vamps_startup.py — Default startup library for VAMPS Python scripting.

This file is auto-loaded from $VAMPSLIB before the user's script.
It provides default at_start / each_step / at_end implementations and
utility functions that mirror the original vamps.sl library.

Users can override at_start / each_step / at_end in their own script
by simply redefining the functions after `import vamps`.
"""

import math
import sys
import os
import time as _time
import vamps

# Add the share directory to sys.path so util.py / regress.py / etc. are
# importable.  Use __file__ (the actual location of this startup script) as
# the authoritative source — works for both installed and source-tree builds
# without requiring $VAMPSLIB to be set in the shell.
_this_dir = os.path.dirname(os.path.abspath(__file__))
if _this_dir not in sys.path:
    sys.path.insert(0, _this_dir)

# Also honour $VAMPSLIB if set and different (e.g. user has a custom lib dir).
_vampslib = os.environ.get("VAMPSLIB", "")
if _vampslib and _vampslib != _this_dir and _vampslib not in sys.path:
    sys.path.insert(0, _vampslib)

# ---------------------------------------------------------------------------
# Utility helpers
# ---------------------------------------------------------------------------

def echo(obj):
    """Print obj to stdout followed by a newline.  Handles scalars and lists."""
    if isinstance(obj, (list, tuple)):
        for row in obj:
            if isinstance(row, (list, tuple)):
                print(" ".join(str(x) for x in row))
            else:
                print(row)
    else:
        print(obj)


def p(obj):
    """Short alias for echo()."""
    echo(obj)


def matrix(m, n):
    """Return an m×n matrix (list of lists) initialised to 0.0."""
    return [[0.0] * n for _ in range(m)]


def linspace(x0, x1, n):
    """Return a list of n linearly spaced values from x0 to x1."""
    if n < 2:
        return [float(x0)]
    inc = (x1 - x0) / (n - 1)
    return [x0 + i * inc for i in range(n)]


def logspace(x0, x1, n):
    """Return a list of n logarithmically spaced values from 10^x0 to 10^x1."""
    if n < 2:
        return [10.0 ** x0]
    inc = (x1 - x0) / (n - 1)
    return [10.0 ** (x0 + i * inc) for i in range(n)]


def transpose(mtx):
    """Transpose a 2-D list-of-lists."""
    return [list(row) for row in zip(*mtx)]


def vamps_progress():
    """Progress indicator — print a dot to stderr."""
    import sys
    sys.stderr.write(".")
    sys.stderr.flush()


# ---------------------------------------------------------------------------
# Default lifecycle callbacks
# (These are intentionally minimal; the user script overrides them.)
# ---------------------------------------------------------------------------

def at_start():
    """Called once before the simulation loop begins."""
    print(f"Run contains {vamps.steps} steps.")


def each_step():
    """Called after every timestep."""
    vamps.printstr("CPU", str(vamps.cpu()))


def at_end():
    """Called once after the simulation loop finishes."""
    vamps.printsum()
    print(f"\nVamps used {vamps.cpu():.3f} CPU seconds")
