Introduction
============

Overview
--------

VAMPS (VAriably saturated soil Model with Plant and canopy System) is a
one-dimensional model for the flow of water through a forested or agricultural
ecosystem.  Development started with two main goals:

1. Make a parameterised model that can adequately describe the flow of water in
   a forested environment (on a plot basis).
2. Be flexible enough to handle the variety of forested environments that exist.
   Because VAMPS is very flexible it can also be applied to plots covered with
   agricultural crops or bare soils.
3. Modelling software should be distributed with complete source code and
   documentation so that users can adapt the program to new situations and
   examine its internal workings.

The flexible nature of VAMPS can be a bit confusing at first because of the
large number of choices available.  The manual and the example files guide
users to a successful application.

Version 1.0 replaces the S-Lang scripting engine with embedded CPython.
Users can hook custom Python functions into a run via ``[vamps] xtrapy`` in
the configuration file.

Source: https://github.com/jaapschellekens/vampspy


Forest water cycle
------------------

The flow of water through a forested ecosystem can be described by three
pathways by which precipitation reaches the forest floor:

* **Direct throughfall** — a small fraction reaches the forest floor without
  touching leaves or stems.
* **Stemflow** — another small fraction flows down tree trunks.
* **Canopy drip / interception evaporation** — the rest hits the forest canopy
  and leaves it as crown drip or evaporation from the wet canopy, depending on
  canopy storage, droplet kinetic energy, and the atmosphere's evaporative
  demand.

Water that infiltrates the soil profile can:

* Leave laterally as saturated or unsaturated flow.
* Percolate to deep groundwater.
* Be extracted by plant roots.

If throughfall intensity exceeds the infiltration capacity of the topsoil,
Hortonian overland flow can occur.  Saturation of the top layer produces
saturation overland flow.  VAMPS simulates most of these fluxes.

The model is mainly driven by atmospheric inputs.  The atmosphere, canopy, and
soil parts can be combined or used separately, although the canopy module is
closely interwoven with the atmosphere module.


Key features
------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Feature
     - Description
   * - Richards equation solver
     - Head-based, adaptive sub-stepping, tridiagonal / band-diagonal / general matrix solvers
   * - Canopy water balance
     - Penman–Monteith transpiration; Gash, Rutter, Calder, and LAI-based interception
   * - Soil hydraulic models
     - Van Genuchten, Clapp/Hornberger, Brooks and Corey, user-defined Python functions, TOPOG tables
   * - Boundary conditions
     - Seven bottom boundary options (free drainage, fixed head/flux, groundwater, seepage, zero flux)
   * - Timestep
     - Variable; can mix daily and sub-daily (e.g. 15-minute) steps within a single run
   * - Python scripting
     - Embedded CPython; hook ``at_start`` / ``each_step`` / ``at_end`` functions via ``[vamps] xtrapy``
   * - vampspy package
     - In-process Python API for single-column and parallel 2-D grid runs (no subprocess, no temp files)
   * - Platforms
     - macOS (arm64, x86_64), Linux (x86_64, arm64)


Model principles
----------------

**Throughfall and interception**

Gross precipitation is partitioned into direct throughfall, stemflow, and
canopy interception.  The preferred method for daily data is the Gash analytical
model :cite:`gash1979165`, which can also be adapted for sub-daily time-steps.
The Rutter numerical model requires hourly or sub-hourly data.  The Calder
regression model and an LAI-based fraction method are also available.

**Transpiration**

Actual transpiration is calculated via the Penman–Monteith combination equation
:cite:`monteith1965`.  During wet-canopy periods (when the canopy storage is not yet
depleted) transpiration is set to zero.  Root water uptake uses the Feddes
or Hoogland stress functions.

**Soil water fluxes**

The soil module solves Richards' equation in pressure-head form with an
adaptive internal sub-stepping scheme.  The number of computational layers
(``[soil] layers``) and the number of physical layers (``[layer_n]`` sections)
are independent; any computation layer not assigned a specific soil section
inherits the settings from the overlying layer.  Lookup tables
(``[soil] mktable = 1``) accelerate the :math:`\theta(h)`, :math:`K(h)`, and
:math:`d\theta/dh` evaluations by up to 50 %.


vampspy Python API
------------------

The ``vampspy`` package drives VAMPS entirely from Python with no subprocess
invocation and no temporary files.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Method
     - Description
   * - ``Model.from_file(path)``
     - Load a ``.inp`` configuration file
   * - ``Model(config, forcing)``
     - Build a model from Python dicts and NumPy arrays
   * - ``model.run()``
     - Whole-run — C drives the timestep loop
   * - ``model.run_stepwise()``
     - Python drives the outer timestep loop; inspect or modify state between steps
   * - ``model.run_grid(forcing_grid)``
     - Parallel multi-column / 2-D grid run; accepts ``(ncols, steps)`` or ``(ny, nx, steps)`` forcing arrays

.. code-block:: python

   from vampspy.model import Model

   m = Model.from_file('examples/fiji/fiji.inp', vampslib='share')
   result = m.run()
   print(result['volact'][-1])   # final water storage [cm]

   # 25x40 grid run
   import numpy as np
   fg = {k: np.tile(v, (25, 40, 1)) for k, v in m.forcing.items()}
   result = m.run_grid(fg, nworkers=4)
   print(result['volact'].shape)  # (25, 40, 61)

See :doc:`vampspy/api` for the full API reference and the ``notebooks/``
directory for worked examples including a 1000-cell grid run.


System requirements
-------------------

* C compiler: gcc or clang (C99)
* GNU make
* Python 3.9 or later (with development headers; ``python3-config`` on PATH)
* NumPy
* termcap or ncurses — required only for interactive binary mode

Tested on macOS (arm64, x86_64) with Apple clang and Linux (x86_64, arm64)
with gcc.
