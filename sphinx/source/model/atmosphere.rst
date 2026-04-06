The Atmosphere
==============

In the atmosphere module, potential transpiration, potential soil
evaporation, and evaporation from a wet surface are calculated from
meteorological and vegetation parameters.  Several methods can be chosen
depending on the available data and the user's preferences.

The terms *potential evaporation*, *actual evaporation*, and
*evapotranspiration* can lead to some confusion, particularly when using
models like VAMPS.  In the final version of the model these will be
simplified; in the meantime most names and units remain as quoted from
the original source.


Potential evaporation
---------------------

Potential evaporation can be used in VAMPS to determine actual evaporation
(or evapotranspiration, depending on the method chosen).  Three methods are
available:

1. Determine Penman :math:`E_0` (open water evaporation) [penman1956N,makkink1957]
   using reflected radiation, net radiation, relative humidity (used to
   calculate the vapour pressure deficit), wind speed, temperature, and
   incoming radiation.
2. Determine Penman :math:`E_0` using sun-ratio, relative humidity, wind speed,
   temperature, and incoming radiation.
3. Using the Makkink formula [makkink1961,commissie1988N] which needs
   incoming radiation, relative humidity, wind speed, and temperature.

The input file settings for potential evaporation are described in the
``[pevaporation]`` section of the :doc:`../user_guide/configuration` chapter.


Penman open-water evaporation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Penman :math:`E_0` is determined via:

.. math::

   E_0 = \frac{\Delta R_{no} + \gamma E_a}{\Delta + \gamma}

Where:

.. list-table::
   :widths: 20 80

   * - :math:`R_{no}`
     - net radiation over open water [cm/day]
   * - :math:`E_a`
     - aerodynamic evaporation or drying power of air [cm/day]
   * - :math:`E_0`
     - open water evaporation [cm/day]
   * - :math:`\Delta`
     - slope of the saturation vapour pressure curve
   * - :math:`\gamma`
     - psychrometer 'constant' [mbar/°K] (an air pressure of 998 mbar is assumed)

The determination of :math:`E_a` follows :cite:`calder1990284`:

.. math::

   E_a = 2.6 \; (e_s - e_a) \; (1 + 0.537 \, u)

Where:

.. list-table::
   :widths: 20 80

   * - :math:`e_a`
     - actual vapour pressure [mbar]
   * - :math:`e_s`
     - vapour pressure at saturation [mbar]
   * - :math:`u`
     - mean daily wind speed at 2 m

VAMPS calculates :math:`e_s` and :math:`e_a` from relative humidity and
dry-bulb temperature using an equation described by :cite:`bringfelt86`.

Net radiation over open water is given by:

.. math::

   R_{no} = R_s (1 - \alpha) - R_{nl}

Where:

.. list-table::
   :widths: 20 80

   * - :math:`R_s`
     - incoming solar radiation
   * - :math:`R_{nl}`
     - net long-wave radiation
   * - :math:`\alpha`
     - albedo of open water (0.05)

In **method 1**, :math:`R_{nl}` is calculated using incoming and reflected
short-wave radiation:

.. math::

   R_{nl} = R_s - R_{net} - Rs_{out}

Where :math:`Rs_{out}` is reflected short-wave radiation and :math:`R_{net}`
is net radiation.

In **method 2**, the sun-ratio :math:`n/N` is used to calculate :math:`R_{nl}`:

.. math::

   R_{nl} = \frac{86400 \, \sigma T^4 \, (0.56 - 0.248 \sqrt{e}) \, (0.1 + 0.9 \, n/N)}{\lambda}

Where:

.. list-table::
   :widths: 20 80

   * - :math:`T`
     - mean daily air temperature [°K]
   * - :math:`e`
     - mean daily water vapour pressure [kPa]
   * - :math:`n/N`
     - ratio of duration of bright sunshine hours :math:`n` to the maximum possible duration of sunshine hours :math:`N`
   * - :math:`\lambda`
     - latent heat of vaporisation


Makkink reference evaporation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Makkink :cite:`commissie1988N` reference evaporation is calculated by:

.. math::

   \lambda E = C \frac{\Delta}{\Delta + \gamma} R_s

Where :math:`C` is a constant (usually 0.65) and :math:`\lambda`, :math:`E`,
:math:`\Delta`, :math:`\gamma`, :math:`R_s` are as defined above.


Actual evapotranspiration
-------------------------

If you do not want to model an entire canopy (see :doc:`canopy`) you can
estimate actual evapotranspiration using one of three methods:

1. Set actual evapotranspiration equal to potential evaporation.
2. Multiply potential evaporation by a crop factor.
3. Calculate actual evaporation using the Penman–Monteith formula.

The preferred approach is to use the full canopy module, which closely
integrates transpiration, interception, and soil evaporation.
