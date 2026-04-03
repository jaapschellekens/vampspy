"""
vmany.py — Extract many soil-profile variables from a VAMPS output file.

Port of vmany.sl.  Reads theta, h, and k time-series from a .out file
and writes each to a tab-separated ASCII matrix file.

Usage::

    python vmany.py run.out
"""
import sys
from util import vsel, f_save


def vmany(infile):
    """Write _theta.mat, _h.mat, and _k.mat from *infile*.

    Parameters
    ----------
    infile : str   path to a VAMPS .out file
    """
    for var, outfile in (("theta", "_theta.mat"), ("h", "_h.mat"), ("k", "_k.mat")):
        print(f"reading {var}...", end="\r", flush=True)
        rows, n = vsel(var, -1, infile)
        if n > 0:
            print(f"writing {outfile}...")
            with open(outfile, "w") as fh:
                for row in rows:
                    fh.write("\t".join(str(v) for v in row) + "\n")
        else:
            print(f"  {var} not found in {infile}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("usage: vmany.py infile.out", file=sys.stderr)
        sys.exit(1)
    vmany(sys.argv[1])
