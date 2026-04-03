"""
_io.py — Low-level I/O helpers for the VAMPS Python interface.

Handles writing .inp config files and time-series forcing files,
and parsing .out result files back into numpy arrays.
"""

from __future__ import annotations
import numpy as np


# ---------------------------------------------------------------------------
# Writing
# ---------------------------------------------------------------------------

def write_ts(arr: np.ndarray, path: str, firststep: int = 1) -> None:
    """Write a 1-D numpy array as a two-column VAMPS time-series file."""
    with open(path, "w") as fh:
        for i, v in enumerate(arr):
            fh.write(f"{firststep + i}\t{v:.8f}\n")


def write_inp_str(config: dict, out_path: str, ts_paths: dict,
                  firststep: int = 1) -> str:
    """Build a VAMPS .inp config as a string (no file written).

    Same semantics as write_inp() but returns the INI text instead of
    writing it to disk.  Use this to pass config directly to the C
    extension without creating a temp file.
    """
    import copy, io
    cfg = copy.deepcopy(config)
    cfg.setdefault("run", {})["outputfile"] = out_path
    cfg.setdefault("time", {}).setdefault("firststep", firststep)
    ts_sec = cfg.setdefault("ts", {})
    for name, path in ts_paths.items():
        ts_sec[name] = path

    lines = []
    for section, params in cfg.items():
        lines.append(f"[{section}]")
        for key, val in params.items():
            if isinstance(val, (list, tuple, np.ndarray)):
                lines.append(_format_array(key, val))
            else:
                lines.append(f"{key} = {val}")
        lines.append("")
    return "\n".join(lines)


def write_inp(config: dict, inp_path: str, out_path: str, ts_paths: dict,
              firststep: int = 1) -> None:
    """Write a VAMPS .inp config file from a nested dict.

    Parameters
    ----------
    config   : nested dict  {section: {key: value}}
    inp_path : destination path for the generated .inp file
    out_path : path where vamps should write the .out file
    ts_paths : {ts_name: file_path} for forcing time-series variables
    firststep: first time-step index written to ts files (default 1)
    """
    import copy
    cfg = copy.deepcopy(config)

    # Inject mandatory run settings
    cfg.setdefault("run", {})["outputfile"] = out_path
    cfg.setdefault("time", {}).setdefault("firststep", firststep)

    # Inject ts file paths (do not overwrite non-array ts entries like hea=hg)
    ts_sec = cfg.setdefault("ts", {})
    for name, path in ts_paths.items():
        ts_sec[name] = path

    lines = []
    for section, params in cfg.items():
        lines.append(f"[{section}]")
        for key, val in params.items():
            if isinstance(val, (list, tuple, np.ndarray)):
                lines.append(_format_array(key, val))
            else:
                lines.append(f"{key} = {val}")
        lines.append("")

    with open(inp_path, "w") as fh:
        fh.write("\n".join(lines))


def _format_array(key: str, vals, width: int = 72) -> str:
    """Format  key = v0 v1 v2 ...  with backslash line continuations."""
    prefix = f"{key} = "
    indent = " " * len(prefix)
    tokens = [str(v) for v in vals]
    result: list[str] = []
    line = prefix
    for tok in tokens:
        if len(line) + len(tok) + 1 > width and line != prefix:
            result.append(line.rstrip() + " \\")
            line = indent
        line += tok + " "
    result.append(line.rstrip())
    return "\n".join(result)


# ---------------------------------------------------------------------------
# Parsing .inp / .out files  (same INI-style format)
# ---------------------------------------------------------------------------

def parse_inp(path: str) -> dict:
    """Parse a VAMPS .inp config file into a nested dict.

    Returns ``{section_name: {key: scalar_or_list}}``.
    Values are converted to int/float where possible; everything else is str.
    Multi-value entries (backslash-continued lines) become lists.
    """
    return _parse_ini(path)


def _parse_ini(path: str) -> dict:
    """Shared INI parser for both .inp and .out files."""
    sections: dict = {}
    current: str | None = None
    current_key: str | None = None
    current_val: list = []

    def flush():
        nonlocal current_key, current_val
        if current is not None and current_key is not None:
            v = current_val
            sections[current][current_key] = v[0] if len(v) == 1 else v
        current_key = None
        current_val = []

    def _conv(s: str):
        try:
            return int(s)
        except ValueError:
            pass
        try:
            return float(s)
        except ValueError:
            return s

    with open(path) as fh:
        for raw in fh:
            line = raw.rstrip("\n")
            stripped = line.strip()

            if stripped.startswith("#") or stripped == "":
                if not (current_key and raw.rstrip().endswith("\\")):
                    flush()
                continue

            if stripped.startswith("[") and stripped.endswith("]"):
                flush()
                current = stripped[1:-1]
                sections.setdefault(current, {})
                continue

            if current is None:
                continue

            # Continuation of a previous multi-token value
            if current_key is not None and "=" not in stripped.split("#")[0]:
                cont = stripped.rstrip("\\").split()
                current_val.extend(_conv(v) for v in cont)
                if not stripped.rstrip().endswith("\\"):
                    flush()
                continue

            if "=" in stripped:
                flush()
                eq = stripped.index("=")
                key = stripped[:eq].strip()
                rest = stripped[eq + 1:].strip()
                if "#" in rest:
                    rest = rest[: rest.index("#")].strip()
                cont_line = rest.endswith("\\")
                if cont_line:
                    rest = rest[:-1].strip()
                current_key = key
                current_val = [_conv(v) for v in rest.split() if v]
                if not cont_line:
                    flush()

    flush()
    return sections


def parse_out(path: str) -> dict:
    """Parse a VAMPS .inp or .out file into a dict of section dicts.

    Returns {section_name: {key: scalar_or_list}}.
    """
    return _parse_ini(path)


def read_out(path: str) -> dict:
    """Parse a VAMPS .out file and return results as numpy arrays.

    Returns a dict where:
      - scalar time-series (one value per step)  → ndarray shape (steps,)
      - profile time-series (one value per layer) → ndarray shape (steps, nlayers)
      - '_initial' key                            → the [initial] section dict
      - '_steps'   key                            → number of timesteps
      - '_t'       key                            → absolute time values ndarray
    """
    sections = parse_out(path)
    initial = sections.get("initial", {})
    steps = int(initial.get("steps", 0))

    per_step: dict[str, list] = {}
    for i in range(steps):
        sec = sections.get(f"t_{i}")
        if sec is None:
            steps = i
            break
        for key, val in sec.items():
            per_step.setdefault(key, []).append(val)

    result: dict = {"_initial": initial, "_steps": steps}

    for key, vals in per_step.items():
        first = vals[0]
        if isinstance(first, list):
            try:
                result[key] = np.array(vals, dtype=float)
            except (ValueError, TypeError):
                result[key] = vals
        else:
            try:
                result[key] = np.array(vals, dtype=float)
            except (ValueError, TypeError):
                result[key] = vals

    # Convenience alias
    if "t" in result:
        result["_t"] = result["t"]

    return result
