Plants and Trees
================

Although VAMPS has been designed to support a multi-layer canopy, this option
is untested and has been disabled in the current version.  At present only one
canopy layer is supported.

At the base of the canopy module lies the Penman–Monteith equation
:cite:`monteith1965`:

.. math::

   \lambda ET = \frac{\Delta (R_n - G) + \rho C_p \, \delta e / r_a}
                     {\Delta + \gamma (1 + r_s / r_a)}

Where:

.. list-table::
   :widths: 20 80

   * - :math:`\rho`
     - density of air
   * - :math:`C_p`
     - specific heat of air
   * - :math:`\delta e`
     - vapour pressure deficit
   * - :math:`\gamma`
     - psychrometric constant
   * - :math:`\Delta`
     - slope of the saturation vapour pressure curve
   * - :math:`r_s`
     - canopy resistance
   * - :math:`r_a`
     - aerodynamic resistance

The program separates wet and dry periods.  Evaporation from the wet canopy
is calculated using the Penman–Monteith equation with :math:`r_s = 0`.  During
these periods transpiration (and hence root extraction) is assumed to be zero.

The amount of net radiation available is determined either via a simple
fraction or via an equation by :cite:`roberts1993197` linking the leaf area profile
to absorbed radiation.

This module combines evaporation, transpiration, and interception, and VAMPS
works best when it is used.


Interception
------------

Interception of precipitation can comprise a large part of the water balance
in forested catchments.  Interception is defined as :cite:`leonard1967184`:

   ... the amount of rainfall retained by the vegetation and evaporated
   without dripping off or running down the stem.  At some point after
   precipitation ceases, all of the intercepted water will be evaporated...

Note that losses in the litter layer are not included.  The balance between
precipitation, evaporation, and canopy storage :cite:`rutter1971174` determines
the amount of rainfall interception.

How interception losses are calculated is mostly determined by the available
data.  The Rutter model :cite:`rutter1971174` needs hourly input, while some of its
parameters should be determined using 5-minute values.  Daily values are most
commonly available, so developing equations for daily values is practical
:cite:`jackson1975183`.  Gash :cite:`gash1979165` developed an analytical variant of the
Rutter model for use with daily data, assuming one storm per day.

The input file settings for interception are described in the
``[interception]`` section of :doc:`../user_guide/configuration`.


Gash's method
~~~~~~~~~~~~~

For daily input data the method described by :cite:`gash1979165` is preferred.
Gash's model is based on the numerical model of :cite:`rutter1971174` but can be
applied to daily measurements assuming one storm per day.  :cite:`gash1980166`
tested both models on the same dataset and found comparable results.

The amount of water needed to completely saturate the canopy, :math:`P'`, is:

.. math::

   P' = \frac{-\overline{R} S}{\overline{E}} \ln \left[
        1 - \frac{\overline{E}}{\overline{R}} (1 - p - p_{Tr})^{-1}
        \right]

Interception losses for storms smaller than :math:`P'`:

.. math::

   E_i = (1 - p - p_{Tr}) P

Interception losses for storms larger than :math:`P'`:

.. math::

   E_i = (1 - p - p_{Tr}) P' + \frac{\overline{E}}{\overline{R}} (P - P')

Where:

.. list-table::
   :widths: 20 80

   * - :math:`\overline{R}`
     - average precipitation [mm/d]
   * - :math:`\overline{E}`
     - average evaporation during storms [mm/d]
   * - :math:`S`
     - storage capacity of the canopy [mm]
   * - :math:`p`
     - free throughfall coefficient
   * - :math:`p_{Tr}`
     - stemflow coefficient
   * - :math:`P`
     - precipitation [mm/d]
   * - :math:`E_i`
     - interception loss [mm]

VAMPS can calculate the ratio :math:`\overline{E}/\overline{R}` from the
Penman–Monteith equation or use a fixed value (generally preferable, as
Penman–Monteith with :math:`R_s = 0` is not very accurate during storms due
to advective energy).

VAMPS also allows the parameters :math:`S`, :math:`p`, and :math:`p_{Tr}` to
be linked to the Leaf Area Index (LAI) using the method of :cite:`dijk1996N`.

A version of the Gash model adapted for sub-daily time-steps is also
available, in which the canopy storage :math:`S` is reduced by the actual
canopy storage at each time-step.  Tests show that results from this adapted
version and the standard daily version differ by no more than 2 %.


Calder regression model
~~~~~~~~~~~~~~~~~~~~~~~

:cite:`calder1986171` presented a simple regression model for interception loss
that can adequately describe interception of forests in the UK:

.. math::

   E_i = \gamma \left[ 1 - \exp(-\delta P) \right]

Where:

.. list-table::
   :widths: 20 80

   * - :math:`E_i`
     - interception loss [mm/d]
   * - :math:`P`
     - precipitation [mm/d]
   * - :math:`\gamma`
     - maximum interception loss per day [mm]
   * - :math:`\delta`
     - fitting parameter


Interception as a fraction of LAI
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. math::

   E_i = \min(P,\; \text{laifrac} \times LAI)

Where:

.. list-table::
   :widths: 20 80

   * - :math:`E_i`
     - interception loss [mm/d]
   * - :math:`P`
     - precipitation [mm/d]
   * - :math:`\text{laifrac}`
     - interception fraction (approximately equal to canopy storage per unit LAI)
   * - :math:`LAI`
     - leaf area index

This method describes total interception well but fails on a storm-by-storm
basis (used in the TOPOG model :cite:`vertessy1993140`).  It is well suited for
applications in which the canopy changes in time.


Rutter's model
~~~~~~~~~~~~~~

:cite:`rutter1971174` developed a numerical interception model based on canopy
storage.  Hourly (preferably 5-minute) meteorological data are required:

.. math::

   dC/dt = P(1 - p - p_{Tr}) - (C/S) E_c - D_r

Where:

.. list-table::
   :widths: 20 80

   * - :math:`t`
     - time-step [hour]
   * - :math:`C`
     - amount of water in the canopy
   * - :math:`P`
     - gross precipitation [mm/h]
   * - :math:`p`
     - free throughfall coefficient
   * - :math:`p_{Tr}`
     - proportion of water discharged via stem flow
   * - :math:`S`
     - canopy storage capacity [mm]
   * - :math:`E_c`
     - evaporation rate from wet canopy [mm/h]
   * - :math:`D_r`
     - drainage speed from storage in canopy [mm/h]

The model is rather insensitive to both :math:`S` and :math:`p`
:cite:`gash1978167`.
