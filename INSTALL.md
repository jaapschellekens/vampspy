# Installation

## Requirements

- ANSI C compiler (gcc ≥ 4 or Apple clang)
- GNU make
- Python ≥ 3.9 with development headers (`python3-config` must be on PATH)
- NumPy (`pip install numpy`)
- termcap or ncurses — for interactive binary mode only; not needed for `vampspy`

**macOS (Homebrew):**
```sh
brew install python
```

**Debian/Ubuntu:**
```sh
apt install gcc make python3-dev
```

---

## Source build

### 1. Configure

```sh
cd src
./configure
```

Optionally set `CFLAGS` before configure for optimised builds:

```sh
export CFLAGS="-O3 -ffast-math"
```

### 2. Build

```sh
make all
```

Builds five static libraries into `libDarwin_arm64/` (or the OS-appropriate
sibling directory):

| Library | Source |
|---------|--------|
| `libdef.a` | `deffile.lib/` |
| `libnru.a` | `nr_ut.lib/` |
| `libmaq.a` | `maq.lib/` |
| `libts.a` | `ts.lib/` |
| `libmet.a` | `met.lib/` |

Then links two executables in `src/`:

- `vamps` — main simulation binary
- `vsel` — time-series extraction utility

### 3. Install (optional)

```sh
make install    # installs to /usr/local (requires write access)
```

Or simply add `src/` to your `PATH`.

### 4. Set VAMPSLIB

```sh
export VAMPSLIB=/path/to/vamps/share
# or from the source tree:
export VAMPSLIB=$(pwd)/share
```

### 5. Install the vampspy Python package

```sh
pip install -e .
```

This compiles the full C source tree with `-DVAMPS_EXT_BUILD` and links it
into `vampspy/_vampscore.so`.

---

## Quick test

```sh
cd examples/fiji
vamps fiji.inp      # binary
python fiji.py      # vampspy Python API
```

---

## Man pages

```sh
make man-install    # run from doc/
```

Or copy `doc/man/vamps.1` and `doc/man/vamps.5` to your man path manually.

---

## Clean

```sh
make -C src clean        # remove object files
make -C src distclean    # remove object files, libraries, and vconfig.h
```
