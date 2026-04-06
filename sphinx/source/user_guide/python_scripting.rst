Python Scripting
================

.. note::

   In VAMPS version 1.0, the S-Lang scripting engine has been removed and
   replaced by embedded CPython.  S-Lang scripts (``.sl`` files) and the
   S-Lang interactive interface are no longer supported.  All scripting is now
   done in Python 3.

Overview
--------

VAMPS embeds a CPython interpreter that is loaded at startup.  Users can hook
custom Python functions into a run by setting ``xtrapy`` in the ``[vamps]``
section of the input file:

.. code-block:: ini

   [vamps]
   xtrapy = myscript.py

The path is resolved relative to the ``.inp`` file.

The script may define any combination of the following three hook functions:

.. code-block:: python

   def at_start():   ...   # called once before the first timestep
   def each_step():  ...   # called after every timestep
   def at_end():     ...   # called after the last timestep

Import the ``vamps`` module to access simulation state:

.. code-block:: python

   import vamps

   def at_end():
       vamps.printsum()
       print(f"CPU: {vamps.cpu():.3f}s")


The ``vamps`` module
--------------------

The ``vamps`` module is provided by the embedded interpreter and exposes
simulation state and control functions to user scripts.  Key functions:

``vamps.printsum()``
    Print a water balance summary to stdout.

``vamps.cpu()``
    Return elapsed CPU time in seconds.

Additional functions for accessing state arrays (``theta``, ``head``, ``k``,
etc.) are available and listed in the ``share/vamps_startup.py`` file that is
auto-loaded before every user script.


Python library modules (``share/``)
-------------------------------------

The following Python modules are provided in the ``share/`` directory and are
available to user scripts via ``sys.path``:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Module
     - Purpose
   * - ``vamps_startup.py``
     - Auto-loaded before every user script; provides defaults and adds ``$VAMPSLIB`` to ``sys.path``
   * - ``util.py``
     - ``.out`` file parser, ``vsel()``, ``vprof()``, ``f_save()``
   * - ``regress.py``
     - Surface resistance regression (``estrs``)
   * - ``met.py``
     - Meteorological functions (Penman–Monteith helpers)
   * - ``runut.py``
     - Run utilities: ``savets``, ``showprof``, ``saveprof``
   * - ``soilf.py``
     - Clapp/Hornberger soil model (soil ``method = 5``)
   * - ``soilf_bc.py``
     - Brooks–Corey soil model (soil ``method = 5``)
   * - ``topsys.py``
     - Scripted top-system (top ``system = 6``)
   * - ``stop.py``
     - Mass-balance check helper
   * - ``stats.py``
     - Column statistics: ``mean``, ``sdev``, ``linreg``, etc.
   * - ``rep.py``
     - Human-readable report generator for ``.out`` files
   * - ``cat.py``, ``sec.py``, ``vmany.py``, ``vsel.py``, ``site.py``
     - Minor utilities


User-defined soil functions (method = 5)
-----------------------------------------

Setting ``method = 5`` in a ``[soilsectionname]`` section allows the user to
replace the built-in soil hydraulic functions with Python callables.  The
following functions must be defined in the script pointed to by ``xtrapy``:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Function
     - Description
   * - ``getspars(section, nr)``
     - Called to read additional parameters from the input file for soil type ``nr``
   * - ``h2t(nr, head)``
     - Moisture content from pressure head
   * - ``t2k(nr, theta)``
     - Unsaturated hydraulic conductivity :math:`K_{unsat}` from moisture content
   * - ``t2h(nr, theta, depth)``
     - Pressure head from moisture content
   * - ``h2dmc(nr, head)``
     - Differential moisture capacity from pressure head
   * - ``h2k(nr, head)``
     - Hydraulic conductivity from pressure head
   * - ``h2u(nr, head)``
     - Volumetric water content from pressure head (integrated)
   * - ``h2dkdp(nr, head)``
     - Derivative of :math:`K` with respect to pressure head

It is strongly recommended to also set ``mktable = 1`` in ``[soil]`` when
using method 5.  Pre-computing lookup tables at initialisation avoids calling
Python on every sub-step and gives performance comparable to the built-in C
functions.

Reference implementations:

* ``share/soilf.py`` — Clapp/Hornberger soil model
* ``share/soilf_bc.py`` — Brooks–Corey soil model

These files can be used as starting points for custom soil models.  Both
implement the full set of required functions and include inline comments
explaining the equations.

Example: using ``soilf.py`` for Clapp/Hornberger soil in method 5:

.. code-block:: ini

   [vamps]
   xtrapy = soilf.py

   [st_0]
   method  = 5
   thetas  = 0.45
   b       = 7.12
   psisat  = -12.0
   ksat    = 100.0

Example: using ``soilf_bc.py`` for Brooks–Corey soil in method 5:

.. code-block:: ini

   [vamps]
   xtrapy = soilf_bc.py

   [st_0]
   method         = 5
   thetas         = 0.6
   theta_residual = 0.08
   lambda         = 0.098
   hb             = -16.4
   ksat           = 1800


vampspy and Python scripting
-----------------------------

When using the ``vampspy`` Python package (see :doc:`../vampspy/api`) rather
than the binary, Python scripting via ``xtrapy`` is still supported.
Additionally, ``vampspy`` provides the ``Model.run_stepwise()`` method which
lets Python code inspect and modify simulation state between external
timesteps, offering a higher-level alternative to hook functions.

See :doc:`../vampspy/api` for the full ``vampspy`` API reference.
