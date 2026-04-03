"""
runut.py — Run utilities for VAMPS Python scripting.

Port of runut.sl.  Provides dataset inspection and saving functions.

Functions
---------
v_show_data_sets()              print a list of all loaded input datasets
v_save_all_sets()               dump all datasets to <name>.set files
savets(tsname, fname, points)   save one time-series to an ASCII file
showprof(par, layers)           print a soil profile array
saveprof(par, layers, z, fname) save a soil profile array to a file
"""

import sys
import vamps


def v_show_data_sets():
    """Print name and source file for every loaded input dataset."""
    for i in range(vamps.sets):
        name  = vamps.getset_name(i)
        fname = vamps.getset_fname(i)
        print(f"Set {i} = {name} read from {fname}")


def v_save_all_sets():
    """Save every loaded dataset to a file named '<datasetname>.set'."""
    for i in range(vamps.sets):
        name  = vamps.getset_name(i)
        fname = f"{name}.set"
        print(f"Saving: {fname}", file=sys.stderr)
        savets(name, fname, vamps.getset_points(i))


def savets(tsname, fname, points):
    """Save a time-series to an ASCII file (two columns: x, y).

    Parameters
    ----------
    tsname : str   name of the input dataset (e.g. "pre", "tem")
    fname  : str   output file path
    points : int   number of points to write

    Returns
    -------
    0 on success, -1 if dataset not found
    """
    setnum = vamps.getsetbyname(tsname)
    if setnum == -1:
        print(f"savets: no such time series: {tsname}", file=sys.stderr)
        return -1

    with open(fname, 'w') as fh:
        fh.write(f"# Name of this set: {tsname}\n")
        for i in range(points):
            x = vamps.getset_x(setnum, i)
            y = vamps.getset_y(setnum, i)
            fh.write(f"{x:f}\t{y:f}\n")
    return 0


def showprof(par, layers):
    """Print a soil profile (depth, value) to stdout.

    Parameters
    ----------
    par    : list of float   values per layer
    layers : int             number of layers
    """
    for i in range(layers):
        print(f"{i}\t{par[i]:f}")


def saveprof(par, layers, z, fname):
    """Save a soil profile to an ASCII file (depth, value).

    Parameters
    ----------
    par    : list of float   values per layer
    layers : int             number of layers
    z      : list of float   depth of each layer mid-point
    fname  : str             output file path
    """
    with open(fname, 'w') as fh:
        for i in range(layers):
            fh.write(f"{z[i]:f}\t{par[i]:f}\n")
