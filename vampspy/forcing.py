"""
forcing.py — Load VAMPS forcing time-series files into numpy arrays.

VAMPS forcing files are simple two-column whitespace-delimited text:

    1.0    12.5
    2.0    14.2
    ...

The first column is the time coordinate; the second is the value.
"""

from __future__ import annotations
import numpy as np


def read_ts(path: str) -> np.ndarray:
    """Read a 2-column VAMPS time-series file.

    Returns the y-values (second column) as a 1-D float64 array.
    """
    data = np.loadtxt(path)
    if data.ndim == 1:
        # Only one row: np.loadtxt returns a 1-D array [x, y]
        return data[1:2]
    return data[:, 1].copy()


def read_ts_timed(path: str) -> tuple[np.ndarray, np.ndarray]:
    """Read a 2-column VAMPS time-series file.

    Returns ``(times, values)`` as two 1-D float64 arrays.
    """
    data = np.loadtxt(path)
    if data.ndim == 1:
        return data[0:1].copy(), data[1:2].copy()
    return data[:, 0].copy(), data[:, 1].copy()


def load_ts_spec(spec: str, base_dir: str) -> "np.ndarray | None":
    """Load a forcing variable from a VAMPS [ts] spec string.

    The spec is whatever appears on the right-hand side of a ``[ts]`` entry:

    * ``"precip.prn"``          — 2-column file; returns the value column.
    * ``"all.inp,0,2"``         — multi-column file; xcol=0 (time), ycol=2.
    * ``"hg"`` or any non-file  — special keyword handled by C; returns None.

    Parameters
    ----------
    spec     : str  — raw value from the [ts] section.
    base_dir : str  — directory of the .inp file (used to resolve relative paths).

    Returns
    -------
    1-D float64 ndarray, or None if spec is not a loadable file.
    """
    import os
    spec = str(spec).strip()
    parts = [p.strip() for p in spec.split(",")]
    filename = parts[0]

    full_path = os.path.join(base_dir, filename)
    if not os.path.isfile(full_path):
        return None  # special keyword (e.g. "hg") — let C handle it

    xcol = int(parts[1]) if len(parts) >= 2 else 0
    ycol = int(parts[2]) if len(parts) >= 3 else 1

    raw = np.loadtxt(full_path, comments="#")
    if raw.ndim == 1:
        raw = raw.reshape(1, -1)
    return raw[:, ycol].copy()


def load_forcing_dir(directory: str, names: list[str],
                     suffix: str = ".prn") -> dict[str, np.ndarray]:
    """Load several forcing variables from a directory.

    Parameters
    ----------
    directory : str
        Directory containing the forcing files.
    names : list[str]
        Variable names to load (e.g. ``['pre', 'nra', 'tem']``).
        The file for each name is ``<directory>/<name><suffix>``.
    suffix : str
        File extension, default ``.prn``.

    Returns
    -------
    dict mapping variable name → 1-D float64 array of values.
    """
    import os
    forcing = {}
    for name in names:
        path = os.path.join(directory, f"{name}{suffix}")
        forcing[name] = read_ts(path)
    return forcing
