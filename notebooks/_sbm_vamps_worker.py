"""Worker module for spawned VAMPS process (cannot be defined inside a notebook cell)."""
import sys
import os
import pickle

def run_vamps(inp_path, result_path):
    sys.path.insert(0, os.path.join(os.path.dirname(inp_path), '..', '..'))
    from vampspy import Model
    import numpy as np
    os.chdir(os.path.dirname(inp_path))
    m = Model.from_file(os.path.basename(inp_path))
    r = m.run_stepwise(firststep=1.0)
    with open(result_path, 'wb') as f:
        pickle.dump(
            {k: np.array(v) for k, v in r.items()
             if k in ['t', 'volact', 'SMD', 'qtop', 'qbot', 'avgtheta',
                      'transpiration', 'soilevaporation', 'interception',
                      'theta', 'h', 'k', 'q']},
            f,
        )
