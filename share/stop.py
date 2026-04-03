"""
stop.py — Mass-balance watchdog for VAMPS Python scripting.

Port of stop.sl.  Import and call stop() from your each_step() hook.

Example::

    from stop import stop

    def each_step():
        stop()
        vamps.printstr("CPU", str(vamps.cpu()))

The default threshold is 3.0 (cm); override by setting stop.maxerr.
"""

import sys
import vamps

#: Mass-balance error threshold that triggers a warning.
maxerr = 3.0


def stop():
    """Warn (to stderr) if |masbal| exceeds *maxerr*.

    In the original S-Lang version this also switched to interactive mode;
    here it prints a prominent warning and raises RuntimeError so the
    caller can decide whether to abort or continue.
    """
    if abs(vamps.masbal) > maxerr:
        sys.stderr.write("\a" * 10 + "\n")
        print(
            f"Warning: mass balance error {vamps.masbal:.4f} > {maxerr},"
            " check input!",
            file=sys.stderr,
        )
        raise RuntimeError(
            f"vamps: mass balance error {vamps.masbal:.4f} exceeds threshold "
            f"{maxerr} at t={vamps.t:.4f}"
        )
