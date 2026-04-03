"""
site.py — Site-specific VAMPS Python settings.

Port of site.sl.  Sets AGL3CONFIG from $VAMPSLIB if the AGL
library is present, so that downstream code can find its config.
"""
import os

vampslib = os.environ.get("VAMPSLIB", "")
if vampslib:
    os.environ.setdefault("AGL3CONFIG", vampslib.rstrip("/") + "/")
else:
    os.environ.setdefault("AGL3CONFIG", "/usr/local/share/vamps1.0/")
