C Source Documentation
======================

The VAMPS C source is documented with embedded doc-comments converted to
Doxygen format by a pre-processing filter.  Run ``make doxygen`` (or the
Sphinx ``make html`` target, which runs Doxygen automatically) to generate
the XML that populates this page.

.. contents:: On this page
   :local:
   :depth: 1

Soil module
-----------

The soil module implements the Richards-equation solver together with
root-extraction, boundary conditions, output collection, and utility routines.

.. doxygenfile:: soil/swatsoil.c
   :project: VAMPS

.. doxygenfile:: soil/rootex.c
   :project: VAMPS

.. doxygenfile:: soil/headcalc.c
   :project: VAMPS

.. doxygenfile:: soil/fluxes.c
   :project: VAMPS

.. doxygenfile:: soil/soilout.c
   :project: VAMPS

.. doxygenfile:: soil/calcgwl.c
   :project: VAMPS

.. doxygenfile:: soil/timestep.c
   :project: VAMPS

.. doxygenfile:: soil/soilboun.c
   :project: VAMPS

.. doxygenfile:: soil/filltab.c
   :project: VAMPS

.. doxygenfile:: soil/reduceva.c
   :project: VAMPS

Meteorological library
----------------------

Functions for computing evapotranspiration components (Penman, Makkink,
Priestley–Taylor) and supporting meteorological quantities.

.. doxygenfile:: met.lib/lambda.c
   :project: VAMPS

.. doxygenfile:: met.lib/gamma.c
   :project: VAMPS

.. doxygenfile:: met.lib/vslope.c
   :project: VAMPS

.. doxygenfile:: met.lib/earo.c
   :project: VAMPS

.. doxygenfile:: met.lib/eaes.c
   :project: VAMPS

.. doxygenfile:: met.lib/e0.c
   :project: VAMPS

.. doxygenfile:: met.lib/makkink.c
   :project: VAMPS

.. doxygenfile:: met.lib/penmon.c
   :project: VAMPS

.. doxygenfile:: met.lib/ra.c
   :project: VAMPS

.. doxygenfile:: met.lib/rn_open.c
   :project: VAMPS
