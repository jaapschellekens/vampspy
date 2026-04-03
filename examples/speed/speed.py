"""
speed.py — Python script for speed.inp

Replaces speed.sl.  Non-interactive: fills the speed.txt columns
automatically from platform info instead of asking via prompts.

Original 8-column format:
  mktable, system_name, cpu_name, mhz, cpu_manufacturer, os_name,
  cpu_seconds, iterations_per_second
"""
import vamps
import platform

def at_start():
    vamps.verbose  = 0
    vamps.soilverb = 1
    print("-" * 67)
    print("Vamps example to determine calculation speed of your setup.")
    print("It uses a fixed dt of 1.0E-4.")
    print("Results are stored in speed.txt")
    print("-" * 67)

def each_step():
    pass

def at_end():
    cpu_used  = vamps.cpu()
    iters_sec = (vamps.steps / 1.0e-4) / cpu_used if cpu_used > 0 else 0.0

    print(f"Vamps used {cpu_used:.3f} CPU seconds")
    print(f"({iters_sec:.1f} iterations/sec)")

    # Reconstruct the original 8-column row as closely as possible
    # without interactive prompts.
    uname      = platform.uname()
    mktable    = 1          # mktable setting from speed.inp [soil] section
    sys_name   = uname.node
    cpu_name   = uname.processor or uname.machine
    mhz        = _cpu_mhz()
    cpu_man    = _cpu_manufacturer()
    os_name    = f"{uname.system}_{uname.machine}"

    with open("speed.txt", "a") as fp:
        fp.write(f"#Vamps 1.0 (Python) [{platform.platform()}]\n")
        fp.write(
            f"{mktable},\t{sys_name},\t{cpu_name},\t{mhz},\t"
            f"{cpu_man},\t\t{os_name},\t{cpu_used:.6f},\t{iters_sec:.2f}\n"
        )
    print("Results appended to speed.txt")


def _cpu_mhz():
    """Best-effort CPU frequency in MHz."""
    try:
        import psutil
        freq = psutil.cpu_freq()
        if freq:
            return f"{freq.max:.0f}"
    except Exception:
        pass
    # Fallback: parse /proc/cpuinfo on Linux
    try:
        with open("/proc/cpuinfo") as f:
            for line in f:
                if "cpu MHz" in line:
                    return line.split(":")[1].strip().split(".")[0]
    except Exception:
        pass
    # macOS sysctl
    try:
        import subprocess
        out = subprocess.check_output(
            ["sysctl", "-n", "hw.cpufrequency_max"], stderr=subprocess.DEVNULL
        ).decode().strip()
        return str(int(out) // 1_000_000)
    except Exception:
        pass
    return "?"


def _cpu_manufacturer():
    """Best-effort CPU manufacturer string."""
    uname = platform.uname()
    proc  = (uname.processor or uname.machine).lower()
    if "apple" in proc or uname.system == "Darwin":
        return "Apple"
    if "intel" in proc:
        return "Intel"
    if "amd" in proc:
        return "AMD"
    if "arm" in proc or "aarch" in proc:
        return "ARM"
    return "?"
