"""
cat.py — Append one ASCII file to another.

Port of cat.sl.

Usage::

    from cat import cat
    cat("new_data.txt", "combined.txt")
"""


def cat(inf, outf):
    """Append the contents of *inf* to *outf* (creating *outf* if needed).

    Parameters
    ----------
    inf  : str   source file path
    outf : str   destination file path (opened in append mode)
    """
    try:
        with open(inf) as src, open(outf, "a") as dst:
            dst.write(src.read())
    except OSError:
        pass
