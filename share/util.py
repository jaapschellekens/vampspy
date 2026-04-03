"""
util.py — Output-file utilities for VAMPS Python scripting.

Port of util.sl.  Provides vsel(), vprof(), and f_save() for reading
data from VAMPS .out files.

The .out file is an INI-like format:

    [section_name]
    key=scalar_value
    key=v0 v1 v2 ...   (array, may span lines with a trailing backslash)

Top-level sections of interest:
    [initial]  — run metadata (steps, z, ...)
    [t_0], [t_1], ...  — per-timestep data

Usage:

    from util import vsel, vprof, f_save

    data, n = vsel("volact", -1, "run.out")
    # data is a list of lists: [[t0, v0, v1, ...], [t1, ...], ...]

    prof, n = vprof("theta", 5, "run.out")
    # prof is a list of [value, depth] pairs for timestep 5

    f_save("volact", "run.out", "volact.dat")
"""

import sys


def _parse_out(filename):
    """Parse a VAMPS .out file into a dict of section dicts.

    Returns {section_name: {key: value_or_list}}.
    Scalar values are returned as float (or int where possible);
    array values as lists of float.
    """
    sections = {}
    current = None
    current_key = None
    current_val = []

    def flush():
        nonlocal current_key, current_val
        if current is not None and current_key is not None:
            vals = current_val
            if len(vals) == 1:
                sections[current][current_key] = vals[0]
            else:
                sections[current][current_key] = vals
        current_key = None
        current_val = []

    def _convert(s):
        try:
            return int(s)
        except ValueError:
            pass
        try:
            return float(s)
        except ValueError:
            return s

    with open(filename) as fh:
        for raw in fh:
            line = raw.rstrip('\n')

            # Comment or blank
            stripped = line.strip()
            if stripped.startswith('#') or stripped == '':
                if not (current_key and current_val and raw.rstrip().endswith('\\')):
                    flush()
                continue

            # Section header
            if stripped.startswith('[') and stripped.endswith(']'):
                flush()
                current = stripped[1:-1]
                sections.setdefault(current, {})
                continue

            if current is None:
                continue

            # Continuation line (previous line ended with \)
            if current_key is not None and not ('=' in stripped.split('#')[0]):
                cont = stripped.rstrip('\\').split()
                current_val.extend(_convert(v) for v in cont)
                if not stripped.rstrip().endswith('\\'):
                    flush()
                continue

            # key=value line
            if '=' in stripped:
                flush()
                eq = stripped.index('=')
                key = stripped[:eq].strip()
                rest = stripped[eq+1:].strip()
                # Strip inline comment
                if '#' in rest:
                    rest = rest[:rest.index('#')].strip()
                # Continuation?
                cont_line = rest.endswith('\\')
                if cont_line:
                    rest = rest[:-1].strip()
                current_key = key
                current_val = [_convert(v) for v in rest.split() if v]
                if not cont_line:
                    flush()

    flush()
    return sections


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def vsel(parname, layer, filename):
    """Read a time-series variable from a VAMPS output file.

    Parameters
    ----------
    parname : str
        Name of the variable (e.g. "volact", "theta").
    layer : int
        Layer index (0-based) to extract, or -1 for all layers.
    filename : str
        Path to the .out file.

    Returns
    -------
    (data, n) where data is a list of rows and n is the number of rows.
    Each row is [t, v] (layer >= 0) or [t, v0, v1, ...] (layer == -1).
    Returns (-1, 0) on failure.
    """
    try:
        sects = _parse_out(filename)
    except OSError as e:
        print(f"vsel: cannot open {filename}: {e}", file=sys.stderr)
        return -1, 0

    init = sects.get("initial", {})
    steps = int(init.get("steps", 0))
    if steps == 0:
        print(f"vsel: 'steps' not found in {filename}", file=sys.stderr)
        return -1, 0

    rows = []
    for i in range(steps):
        sec = sects.get(f"t_{i}")
        if sec is None:
            break
        t_val = float(sec.get("t", i))
        val = sec.get(parname)
        if val is None:
            break
        if not isinstance(val, list):
            val = [val]
        if layer == -1:
            rows.append([t_val] + [float(v) for v in val])
        else:
            if layer < len(val):
                rows.append([t_val, float(val[layer])])
            else:
                print(f"vsel: layer {layer} out of range for {parname}", file=sys.stderr)
                break

    if not rows:
        print(f"vsel: variable '{parname}' not found in {filename}", file=sys.stderr)
        return -1, 0

    return rows, len(rows)


def vprof(var, timestep, filename):
    """Read a soil profile variable for one timestep.

    Parameters
    ----------
    var : str
        Variable name (e.g. "theta").
    timestep : int
        Timestep index (0-based).
    filename : str
        Path to the .out file.

    Returns
    -------
    (profile, n) where profile is a list of [value, depth] pairs and
    n is the number of layers. Returns (-1, 0) on failure.
    """
    try:
        sects = _parse_out(filename)
    except OSError as e:
        print(f"vprof: cannot open {filename}: {e}", file=sys.stderr)
        return -1, 0

    init = sects.get("initial", {})
    z_raw = init.get("z")
    if z_raw is None:
        print(f"vprof: 'z' not found in [initial] of {filename}", file=sys.stderr)
        return -1, 0
    z = z_raw if isinstance(z_raw, list) else [z_raw]

    sec = sects.get(f"t_{timestep}")
    if sec is None:
        print(f"vprof: section t_{timestep} not found in {filename}", file=sys.stderr)
        return -1, 0

    val = sec.get(var)
    if val is None:
        print(f"vprof: variable '{var}' not found in t_{timestep} of {filename}", file=sys.stderr)
        return -1, 0

    if not isinstance(val, list):
        val = [val]

    n = min(len(z), len(val))
    profile = [[float(val[i]), float(z[i])] for i in range(n)]
    return profile, n


def f_save(var, infile, outfile):
    """Write a time-series variable from a .out file to an ASCII file.

    Parameters
    ----------
    var : str
        Variable name.
    infile : str
        Path to the VAMPS .out file.
    outfile : str
        Destination ASCII file (columns separated by tabs).
    """
    result, n = vsel(var, -1, infile)
    if result == -1:
        print(f"f_save: reading '{var}' from {infile} failed.", file=sys.stderr)
        return
    with open(outfile, 'w') as fh:
        for row in result:
            fh.write('\t'.join(str(v) for v in row) + '\n')
