*Pinus caribaea* Plantation Forest — Fiji
==========================================

Introduction
------------

This chapter presents the results of running VAMPS on a dataset concerning
a *Pinus caribaea* plantation forest located on Viti Levu, Fiji
:cite:`waterloo1994T2`.  A 61-day period (2 July — 2 September) from the Tulasewa
site was modelled using meteorological data with 30-minute resolution.

The soils at this site have a bulk density ranging from 0.97 at the top to
1.07 g/cm at 1.2 m below the surface.  Saturated hydraulic conductivity was
determined for three soil layers from 29 core samples (Table 1).
:cite:`waterloo1994T2` determined Van Genuchten parameters for the topsoil (0–30 cm)
and the subsoil (30–150 cm) using 12 and 17 samples respectively.  These
values were used in the soil section of VAMPS without any modification.

Transpiration was modelled using the Penman–Monteith combination equation
while interception was modelled using an adapted version of the Gash model
(described in the VAMPS model documentation).  Soil evaporation was also
modelled using the Penman–Monteith combination equation.  It was assumed that
net radiation at the forest floor was 3.5 % of that at the top of the canopy.

Measured values of moisture content (from a capacitance probe) were used to
verify model results.


Soil parameters — Tulasewa site
--------------------------------

Van Genuchten parameters used as VAMPS input (after :cite:`waterloo1994T2`):

.. list-table::
   :header-rows: 1
   :widths: 20 15 15 20 15 15

   * - Soil layer
     - Porosity
     - Alpha [1/cm]
     - :math:`K_{sat}` [cm/day]
     - n
     - Depth [cm]
   * - 1
     - 0.60
     - 0.061
     - 1800
     - 1.098
     - 0 – 30
   * - 2
     - 0.61
     - 0.042
     - 380
     - 1.094
     - 30 – 75
   * - 3
     - 0.61
     - 0.042
     - 0.03
     - 1.094
     - 70 – 150


Results
-------

The water balance summary for the modelled period is shown below:

.. list-table::
   :header-rows: 1
   :widths: 60 40

   * - Parameter
     - Value [cm]
   * - Initial water volume of profile
     - 71.765
   * - Saturated water volume of profile
     - 98.397
   * - Total precipitation
     - 25.767
   * - Total transpiration
     - 16.753
   * - Total interception
     - 5.261
   * - Total soil evaporation
     - 1.522
   * - Total root extraction
     - 10.693
   * - Total drainage
     - −0.008
   * - Total surface runoff
     - 0.000
   * - Initial storage
     - 71.765
   * - Final storage
     - 80.197
   * - Change in storage
     - −8.432
   * - Percent mass-balance error
     - −0.039 %

The modelled interception and transpiration values are similar to those
presented by :cite:`waterloo1994T2`.  Canopy resistance (:math:`R_s`) was calculated
using the relations derived by :cite:`waterloo1994T2`.  Calculated potential
transpiration could not be maintained over the period due to modelled water
stress; calculated root extraction (actual modelled transpiration) was 5.5 cm
lower.  This may be caused by:

1. The regression equations used to estimate :math:`R_s` were obtained for a
   wetter period in which the trees did not suffer water stress.
2. The water content versus suction-head functions are not very accurate in
   the drier regions and may overestimate the suction head.

The average moisture content is generally modelled with good result.  The
upper layers show low moisture contents (theta ≤ 0.3); the pF corresponding
to a moisture content of 0.3 is already beyond 4.2.


Example input files
-------------------

The example input files for this case study are:

* ``examples/fiji/fiji.inp`` — original run using Van Genuchten parameters.
* ``examples/fiji/fiji_bandC.inp`` — equivalent run using Brooks–Corey
  parameters derived from the Van Genuchten values (λ ≈ n − 1,
  :math:`h_b \approx -1/\alpha`).  This file demonstrates method 6 and can
  be used to compare the two retention curve approaches.

The input file used for the Fiji study (abbreviated):

.. code-block:: ini

   [vamps]
   iniinmem=1

   [run]
   outputfile = run5.hh.out

   [determine]
   canopy = 1
   soilmoisture = 1

   [time]
   steps = 2930

   [ts]
   precipitation = ../ninp/precip.prn
   netrad        = ../ninp/rnet.prn
   rhumid        = ../ninp/rh.prn
   temp          = ../ninp/newt.prn
   windspeed     = ../ninp/wind.prn

   [interception]
   method  = 0
   E_avg/R = 0.147
   p_f     = 0.6
   p_tr    = 0.017
   S       = 0.08
   St      = 0.0062

   [canopy]
   transpiration  = 2
   Rnet_absorb    = 0.975
   layers         = 1
   z              = 12.7
   z_0            = 1.5
   d              = 7.0

   [roots]
   swsink  = 0
   swhypr  = 0
   swupfu  = 0
   depth   = 120.0
   hlim1   = -5.0
   hlim2u  = -50.0
   hlim2l  = -50.0
   hlim3h  = -800.0
   hlim3l  = -1000.0
   hlim3   = -1800.0
   hlim4   = -12000.0

   [soil]
   layers        = 77
   bottom        = 6
   initprof      = 0
   theta_initial = 0.200000 0.210000 ...  (77 values)

   [layer_0]
   description    = Tulasewa top layer
   thickness      = 2.5
   method         = 1
   thetas         = 0.6
   theta_residual = 0.08
   alpha          = 0.061
   n              = 1.098
   l              = 0.5
   ksat           = 1800

   [layer_14]
   description    = Tulasewa 30-75 cm layer
   thickness      = 2.0
   thetas         = 0.64
   alpha          = 0.042
   n              = 1.094
   l              = 0.5
   ksat           = 380.0

   [layer_36]
   description    = Tulasewa deep layer > 75 cm
   thickness      = 2.0
   thetas         = 0.6
   ksat           = 3.0


Running with Python
--------------------

The same run can be driven entirely from Python using ``vampspy``:

.. code-block:: python

   from vampspy.model import Model

   m = Model.from_file('examples/fiji/fiji.inp', vampslib='share')
   result = m.run()
   print(f"Total precipitation: {result['cumprec'][-1]:.3f} cm")
   print(f"Final soil storage:  {result['volact'][-1]:.3f} cm")
   print(f"Theta array shape:   {result['theta'].shape}")   # (61, 77)

For the Brooks–Corey comparison:

.. code-block:: python

   m_bc = Model.from_file('examples/fiji/fiji_bandC.inp', vampslib='share')
   result_bc = m_bc.run()
