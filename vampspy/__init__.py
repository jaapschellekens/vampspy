"""
vampspy — Python interface to the VAMPS 1D unsaturated zone model.

Quick start::

    from vampspy import Model
    import numpy as np

    model = Model(
        config={...},      # nested dict mirroring a .inp file
        forcing={          # numpy arrays, one per ts variable
            "pre": precip_array,
            ...
        },
        executable="/path/to/vamps",
    )
    result = model.run()
    print(result["volact"])   # ndarray, shape (steps,)
    print(result["theta"])    # ndarray, shape (steps, nlayers)
"""

from vampspy.model import Model
from vampspy import forcing

__all__ = ["Model", "forcing"]
__version__ = "0.1.0"
