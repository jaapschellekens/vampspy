"""
fiji.py — Python script for fiji.inp

Fiji pine forest simulation (61 daily timesteps, 77 soil layers).
Prints a run description, shows a summary at the end, and saves
key time-series to ASCII files for post-processing.
"""
import vamps

def at_start():
    print("-" * 67)
    print("Fiji pine forest — Penman-Monteith canopy, Van Genuchten soil")
    print(f"77 layers, {vamps.steps} daily timesteps")
    print("-" * 67)

def each_step():
    vamps.printstr("CPU", str(vamps.cpu()))

def at_end():
    vamps.printsum()
    print(f"\nVamps used {vamps.cpu():.3f} CPU seconds")

    from util import f_save
    for var in ("volact", "SMD", "qtop", "qbot", "rootextract",
                "cumprec", "cumtra", "cumeva"):
        f_save(var, vamps.outfilename, f"{var}.dat")
    print("Saved: volact  SMD  qtop  qbot  rootextract  cumprec  cumtra  cumeva  (.dat)")
