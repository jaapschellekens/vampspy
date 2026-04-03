"""
sec.py — List keys defined in a named section of a VAMPS INI-style file.

Port of sec.sl.

Usage::

    from sec import sec
    sec("run.out", "[final]")   # section header including brackets

Or from the command line::

    python sec.py run.out [final]
"""
import sys


def sec(filename, section):
    """Print key names (left of '=') found in *section* of *filename*.

    Parameters
    ----------
    filename : str   path to any INI-style file (e.g. a .out or .inp)
    section  : str   section header including brackets, e.g. ``[final]``
    """
    insec = False
    with open(filename) as fh:
        for raw in fh:
            s = raw.strip()
            if s.startswith("[") and s.endswith("]"):
                insec = False
            if s == section:
                insec = True
            if insec and "=" in s:
                key = s.split("=", 1)[0].strip()
                print(key)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("usage: sec.py file [section]", file=sys.stderr)
        sys.exit(1)
    sec(sys.argv[1], sys.argv[2])
