"""
vsel.py — Command-line wrapper for the vsel() time-series extractor.

Port of vsel.sl.  Extracts a named variable from all timesteps of a
VAMPS .out file and writes the result as a tab-separated ASCII matrix.

Usage::

    python vsel.py var infile.out outfile.mat
"""
import sys
from util import vsel as _vsel


def main():
    if len(sys.argv) < 4:
        print("usage: vsel.py var infile outfile", file=sys.stderr)
        sys.exit(1)

    var, infile, outfile = sys.argv[1], sys.argv[2], sys.argv[3]

    rows, n = _vsel(var, -1, infile)
    if n > 0:
        with open(outfile, "w") as fh:
            for row in rows:
                fh.write("\t".join(str(v) for v in row) + "\n")
    else:
        print(f"Var {var} not found in {infile}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
