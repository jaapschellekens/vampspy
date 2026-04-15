BMI Interface
=============

``vampspy`` provides a `Basic Model Interface (BMI) 2.0
<https://bmi.readthedocs.io/>`_ implementation through the
:class:`~vampspy.bmi.VampsBmi` class.  BMI is a standard coupling interface
used by frameworks such as `CSDMS <https://csdms.colorado.edu>`_ and
`OpenDA <https://www.openda.org>`_ to connect hydrology models.

.. contents:: On this page
   :local:
   :depth: 2

Configuration file
------------------

``VampsBmi`` is initialised from a small YAML file that points to a VAMPS
``.inp`` file and optionally overrides a few run-control parameters:

.. code-block:: yaml

   inp_file  : path/to/model.inp   # VAMPS .inp file (required)
   ncols     : 1                   # number of independent soil columns (default 1)
   firststep : 1.0                 # calendar offset for BMI time reporting (default 1.0)
   steps     : 61                  # total timesteps; default taken from [time].steps

All paths are relative to the directory containing the YAML file.

Variable catalogue
------------------

Input variables (set with :meth:`~vampspy.bmi.VampsBmi.set_value` before each
:meth:`~vampspy.bmi.VampsBmi.update`):

.. list-table::
   :header-rows: 1
   :widths: 30 12 15 20

   * - BMI name
     - Grid
     - Units
     - VAMPS ``[ts]`` key
   * - ``precipitation``
     - 0
     - cm d⁻¹
     - ``pre``
   * - ``net_radiation``
     - 0
     - W m⁻²
     - ``nra``
   * - ``incoming_radiation``
     - 0
     - W m⁻²
     - ``ira``
   * - ``relative_humidity``
     - 0
     - — (0–1)
     - ``rlh``
   * - ``air_temperature``
     - 0
     - °C
     - ``tem``
   * - ``wind_speed``
     - 0
     - m s⁻¹
     - ``win``

Output scalars (retrieve with :meth:`~vampspy.bmi.VampsBmi.get_value` after each
:meth:`~vampspy.bmi.VampsBmi.update`):

.. list-table::
   :header-rows: 1
   :widths: 30 12 15 20

   * - BMI name
     - Grid
     - Units
     - VAMPS output key
   * - ``soil_water_volume``
     - 0
     - cm
     - ``volact``
   * - ``soil_moisture_deficit``
     - 0
     - cm
     - ``SMD``
   * - ``surface_runoff``
     - 0
     - cm timestep⁻¹
     - ``qtop``
   * - ``bottom_drainage``
     - 0
     - cm timestep⁻¹
     - ``qbot``
   * - ``avg_soil_water_content``
     - 0
     - —
     - ``avgtheta``
   * - ``precipitation_flux``
     - 0
     - cm timestep⁻¹
     - ``precipitation``
   * - ``transpiration_flux``
     - 0
     - cm timestep⁻¹
     - ``transpiration``
   * - ``soil_evaporation_flux``
     - 0
     - cm timestep⁻¹
     - ``soilevaporation``
   * - ``interception_flux``
     - 0
     - cm timestep⁻¹
     - ``interception``

Output profiles — one value per soil layer (grid 1):

.. list-table::
   :header-rows: 1
   :widths: 30 12 15 20

   * - BMI name
     - Grid
     - Units
     - VAMPS output key
   * - ``soil_water_content``
     - 1
     - —
     - ``theta``
   * - ``pressure_head``
     - 1
     - cm
     - ``h``
   * - ``hydraulic_conductivity``
     - 1
     - cm d⁻¹
     - ``k``

Grid layout
-----------

Two BMI grids are defined:

* **Grid 0** — scalar surface grid, shape ``(ncols,)``.  One node per soil
  column.
* **Grid 1** — soil-profile grid, shape ``(ncols, nlayers)``, stored
  row-major (all layers of column 0 first).  ``nlayers`` is determined by the
  ``layers`` setting in the ``.inp`` file.

Multi-column mode
-----------------

When ``ncols > 1``, each column runs in its own worker process so that the
global C state in ``_vampscore`` remains isolated between columns.  All BMI
variable arrays then have an extra leading dimension of size ``ncols``:

.. code-block:: python

   import numpy as np
   from vampspy.bmi import VampsBmi

   bmi = VampsBmi()
   bmi.initialize("config.yaml")          # ncols: 3 in config.yaml

   precip = np.array([0.5, 0.2, 0.0])    # one value per column
   bmi.set_value("precipitation", precip)
   bmi.update()

   theta = np.zeros(bmi.get_grid_size(1))
   bmi.get_value("soil_water_content", theta)
   theta = theta.reshape(3, -1)           # (ncols, nlayers)

API reference
-------------

.. autoclass:: vampspy.bmi.VampsBmi
   :members:
   :special-members: __init__
   :member-order: bysource
   :show-inheritance:
