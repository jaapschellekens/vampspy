"""
example1.py — Python script for example1.inp

Replaces example1.sl.  Non-interactive: just sets verbosity,
prints a description at start, and prints a summary with key
results at the end.
"""
import vamps

def at_start():
    vamps.verbose   = 0
    vamps.soilverb  = 1
    print("-" * 67)
    print("Simple Vamps example. All top fluxes are precalculated")
    print("(see 'all.inp') and one soil layer is defined.")
    print("-" * 67)
    print(f"Run contains {vamps.steps} steps.")

def each_step():
    pass   # nothing extra per step

def at_end():
    vamps.printsum()
    print(f"\nVamps used {vamps.cpu():.3f} CPU seconds")

    # Save key output variables for post-processing
    from util import f_save
    f_save("volact", vamps.outfilename, "volact.dat")
    f_save("SMD",    vamps.outfilename, "SMD.dat")
    f_save("qbot",   vamps.outfilename, "qbot.dat")
    print("Saved: volact.dat  SMD.dat  qbot.dat")
