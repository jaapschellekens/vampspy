"""
tests/test_bmi_single_column.py

Regression test: VampsBmi (ncols=1, single column, step-by-step) must
produce the same scalar and profile outputs as Model.from_file().run()
(whole-run reference) for the Fiji example.

Both runs use the same C extension (_vampscore) which has global state,
so they are executed in separate worker processes to avoid conflicts.

Run with:
    python tests/test_bmi_single_column.py
or:
    pytest tests/test_bmi_single_column.py -v
"""
from __future__ import annotations

import multiprocessing as mp
import os
import sys
import tempfile

import numpy as np

# Ensure the package is importable when running from repo root
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

FIJI_INP = os.path.abspath(os.path.join(
    os.path.dirname(__file__), "..", "examples", "fiji", "fiji.inp",
))

ATOL = 1e-10
RTOL = 1e-8


# ---------------------------------------------------------------------------
# Worker functions — each runs in its own spawned process
# ---------------------------------------------------------------------------

def _worker_reference(inp_file: str, firststep: float, out_npz: str) -> None:
    """Run Model.from_file().run_stepwise() and save results to out_npz."""
    from vampspy.model import Model
    m = Model.from_file(inp_file)
    result = m.run_stepwise(firststep=firststep)
    # Save only the arrays we need (no special keys starting with _)
    arrays = {k: np.asarray(v) for k, v in result.items() if not k.startswith("_")}
    arrays["__steps"]   = np.array(result["_steps"])
    arrays["__nlayers"] = np.array(result["_nlayers"])
    np.savez(out_npz, **arrays)


def _worker_bmi(inp_file: str, firststep: float, steps: int, out_npz: str) -> None:
    """Run VampsBmi (ncols=1) step-by-step and save results to out_npz."""
    import tempfile
    import yaml
    import numpy as np
    from vampspy.bmi import VampsBmi, _SCALAR_OUTPUTS, _PROFILE_OUTPUTS, _BMI_TO_TS
    from vampspy.forcing import load_ts_spec
    from vampspy._io import parse_inp

    inp_file = os.path.abspath(inp_file)
    cfg = {
        "inp_file":  inp_file,
        "ncols":     1,
        "steps":     steps,
        "firststep": firststep,
    }
    with tempfile.NamedTemporaryFile(mode="w", suffix=".yml", delete=False) as tf:
        yaml.dump(cfg, tf)
        tmp_yml = tf.name

    bmi = VampsBmi()
    try:
        bmi.initialize(tmp_yml)
    finally:
        os.unlink(tmp_yml)

    # Load forcing from .prn files
    inp_dir = os.path.dirname(inp_file)
    ts_section = parse_inp(inp_file).get("ts", {})
    forcing_data: dict[str, np.ndarray] = {}
    for ts_name, spec in ts_section.items():
        arr = load_ts_spec(str(spec), inp_dir)
        if arr is not None:
            forcing_data[ts_name] = arr[:steps]

    active_inputs = set(bmi.get_input_var_names())
    collected_scalar:  dict[str, list] = {n: [] for n in _SCALAR_OUTPUTS}
    collected_profile: dict[str, list] = {n: [] for n in _PROFILE_OUTPUTS}
    dest_scalar  = np.zeros(1)
    dest_profile = np.zeros(bmi._nlayers)

    for i in range(steps):
        for bmi_name in active_inputs:
            ts_name = _BMI_TO_TS[bmi_name]
            if ts_name in forcing_data:
                bmi.set_value(bmi_name, np.array([forcing_data[ts_name][i]]))
        bmi.update()

        for bmi_name in _SCALAR_OUTPUTS:
            bmi.get_value(bmi_name, dest_scalar)
            collected_scalar[bmi_name].append(float(dest_scalar[0]))

        for bmi_name in _PROFILE_OUTPUTS:
            bmi.get_value(bmi_name, dest_profile)
            collected_profile[bmi_name].append(dest_profile.copy())

    bmi.finalize()

    arrays: dict[str, np.ndarray] = {}
    for n in _SCALAR_OUTPUTS:
        arrays[n] = np.array(collected_scalar[n])
    for n in _PROFILE_OUTPUTS:
        arrays[n] = np.stack(collected_profile[n], axis=0)
    np.savez(out_npz, **arrays)


# ---------------------------------------------------------------------------
# Spawn helpers
# ---------------------------------------------------------------------------

def _spawn(target, args):
    """Run *target(*args)* in a fresh spawned process; raise on failure."""
    ctx  = mp.get_context("spawn")
    proc = ctx.Process(target=target, args=args)
    proc.start()
    proc.join()
    if proc.exitcode != 0:
        raise RuntimeError(
            f"{target.__name__} worker exited with code {proc.exitcode}"
        )


# ---------------------------------------------------------------------------
# Main comparison logic
# ---------------------------------------------------------------------------

_CHECKS = [
    # (label,              bmi_name,                  model_key)
    ("volact",             "soil_water_volume",        "volact"),
    ("SMD",                "soil_moisture_deficit",    "SMD"),
    ("qtop",               "surface_runoff",           "qtop"),
    ("qbot",               "bottom_drainage",          "qbot"),
    ("avgtheta",           "avg_soil_water_content",   "avgtheta"),
    ("transpiration",      "transpiration_flux",       "transpiration"),
    ("soilevaporation",    "soil_evaporation_flux",    "soilevaporation"),
    ("interception",       "interception_flux",        "interception"),
    ("theta (profile)",    "soil_water_content",       "theta"),
    ("pressure_head",      "pressure_head",            "h"),
]

FIRSTSTEP = 1.0   # always 1-based internally; calendar offset is BMI-only


def _run_both() -> "tuple[dict, dict]":
    """Run reference and BMI in separate spawned processes, return (ref, bmi)."""
    with tempfile.TemporaryDirectory() as tmp:
        ref_npz = os.path.join(tmp, "ref.npz")
        bmi_npz = os.path.join(tmp, "bmi.npz")

        _spawn(_worker_reference, (FIJI_INP, FIRSTSTEP, ref_npz))
        ref_data = dict(np.load(ref_npz, allow_pickle=False))

        steps = int(ref_data.pop("__steps"))
        ref_data.pop("__nlayers", None)

        _spawn(_worker_bmi, (FIJI_INP, FIRSTSTEP, steps, bmi_npz))
        bmi_data = dict(np.load(bmi_npz, allow_pickle=False))

    return ref_data, bmi_data


def main() -> int:
    print(f"Fiji .inp : {FIJI_INP}")
    if not os.path.isfile(FIJI_INP):
        print("ERROR: fiji.inp not found", file=sys.stderr)
        return 2

    print("Running reference (Model.from_file().run()) in worker process …")
    print("Running BMI      (VampsBmi ncols=1 step-by-step) in worker process …")
    ref, bmi = _run_both()

    print("\nComparing outputs:")
    failures = []
    for label, bmi_name, model_key in _CHECKS:
        if model_key not in ref:
            print(f"  SKIP  {label:30s} (not in reference)")
            continue
        if bmi_name not in bmi:
            print(f"  SKIP  {label:30s} (not in BMI output)")
            continue
        got  = bmi[bmi_name]
        want = ref[model_key]
        if got.shape != want.shape:
            msg = f"shape: got={got.shape} want={want.shape}"
            print(f"  FAIL  {label:30s}  {msg}")
            failures.append(f"{label}: {msg}")
            continue
        if np.allclose(got, want, atol=ATOL, rtol=RTOL, equal_nan=True):
            print(f"  PASS  {label:30s}  max|diff|={np.abs(got-want).max():.2e}")
        else:
            diff = np.abs(got - want)
            idx  = np.unravel_index(np.nanargmax(diff), diff.shape)
            msg  = (f"max|diff|={diff[idx]:.3e} at index {idx}  "
                    f"got={got[idx]:.6g}  want={want[idx]:.6g}")
            print(f"  FAIL  {label:30s}  {msg}")
            failures.append(f"{label}: {msg}")

    print()
    if failures:
        print(f"FAILED — {len(failures)} check(s) failed")
        return 1
    print("ALL PASSED")
    return 0


# ---------------------------------------------------------------------------
# pytest entry points
# ---------------------------------------------------------------------------

def test_bmi_scalar_outputs():
    ref, bmi = _run_both()
    for label, bmi_name, model_key in _CHECKS:
        if "profile" in label:
            continue
        if model_key not in ref or bmi_name not in bmi:
            continue
        got, want = bmi[bmi_name], ref[model_key]
        assert np.allclose(got, want, atol=ATOL, rtol=RTOL), (
            f"{label}: max|diff|={np.abs(got-want).max():.3e}"
        )


def test_bmi_profile_theta():
    ref, bmi = _run_both()
    if "theta" not in ref or "soil_water_content" not in bmi:
        return
    got, want = bmi["soil_water_content"], ref["theta"]
    assert np.allclose(got, want, atol=ATOL, rtol=RTOL), (
        f"theta: max|diff|={np.abs(got-want).max():.3e}"
    )


if __name__ == "__main__":
    sys.exit(main())
