Configuration
=============

Program organisation
--------------------

The main module of VAMPS is the ``vamps`` program itself.  It uses a series of
equations to describe the flow of water through a forested ecosystem.
Preparing input for VAMPS takes quite a while and is by no means a trivial
task.

Post-processing tools included with VAMPS:

* **vsel** — the recommended tool for general post-processing.  It can
  produce data that can be fed directly to plotting programmes such as
  ``gnuplot``.
* ``util.py`` in ``share/`` — Python equivalent providing ``vsel()``,
  ``vprof()``, and ``f_save()``.

VAMPS can crash in certain situations:

1. A genuine bug.
2. The situation to be modelled is beyond the capabilities of VAMPS.
3. The timestep estimate done by VAMPS is not right.  A lot of precipitation
   on a very clayey soil is a recipe for disaster.

Troubleshooting options are described in :ref:`sec-troubles`.


Layers in the soil section
~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``layers`` variable in the ``[soil]`` section determines the number of
layers used in the *calculations* (nodes, strictly speaking).  The number of
*physical* layers (with different :math:`K_{sat}`, for example) is determined
by the number of ``[layer_n]`` sections.  Any computational layer that is not
assigned a specific layer section automatically inherits the settings from the
overlying layer.

The ``[layer_0]`` section is the only one that is mandatory.


Time series and time-steps
~~~~~~~~~~~~~~~~~~~~~~~~~~

Time series are values of variables over time used as input.  All time series
must be listed in the ``[ts]`` section.

The precipitation time series is special because it also determines the
time-steps at which output is calculated.  The x-column should be in days or
fractions of a day (e.g. 1.5 means halfway through day 1).  Time-steps may
vary within one run.  Example precipitation file:

.. code-block:: text

   # This is a comment
   # Precipitation from the no-name site
   # time in days         amounts in cm/day (intensity!)
   0.5     0.6
   1.0     0.9
   2.0     0.7
   2.1     0.9
   2.2     0.7
   2.3     0.5
   3.0     0.8
   3.5     0.0

Points to remember:

1. Units are cm/day (intensities).  Convert precipitation data if necessary.
2. The time in the x-column is the *end* of an interval.  Output at time 3.0
   covers the period from time 2.3 to 3.0 (duration 0.7 days).  The first
   record has no known preceding interval; VAMPS defaults to one day.  You can
   change this with the ``firststep`` variable in the ``[time]`` section.
3. The x-column in all files except the precipitation file is ignored (it must
   be present but VAMPS does not check the time values).


Input file format
-----------------

The VAMPS input file format is similar to Windows ``.ini`` files.  All names
and sections are case-insensitive.  String variable values preserve case, so
filenames are case-sensitive if the operating system is.

The file is divided into sections, each containing variables with a related
purpose.  Each variable name is followed by ``=`` and then the value.  Example:

.. code-block:: ini

   [vamps]
   verbose = TRUE
   # This is a comment

   [environment]
   caseid = Interception test file. Bisley catchment.\
   Summer of 1995

The ``\`` character may be used to break long strings over more than one line
(it is converted to a space).  The ``#`` sign at the start of a line denotes a
comment.  Only the first ``=`` sign is significant.  Spaces within variable
names are preserved verbatim, making the following legal:

.. code-block:: ini

   [vamps]
   output mode = This is ('=') a nonsense example.

VAMPS uses six variable types:

1. **floats** — floating point numbers
2. **arrays** — a row of floating point numbers separated by whitespace
3. **strings** — a series of characters
4. **integers** — whole signed numbers
5. **characters** — a single printable character
6. **boolean** — either 0/1, FALSE/TRUE, or NO/YES


Available sections and variables
---------------------------------

``[vamps]``
~~~~~~~~~~~

``verbose``
    If set to FALSE VAMPS will be silent; if set to TRUE VAMPS will display
    progress information.

``header``
    If set to FALSE no header is added to the output; if set to TRUE a header
    is added.  Default is FALSE.

``logging``
    FALSE = no logging; TRUE = logging is on.

``logfilename``
    Name of the file to which logging is written.

``iniinmem``
    TRUE = the input file is read into memory (some speed-up); FALSE = the
    input file is not read into memory.

``progstr``
    String used to display progress information.  Use 0, 1 or 2 for built-in
    strings: 0 = calculation time and estimated time remaining; 1 = percentage
    finished bar; 2 = 'calculating'.  You can define your own string.

``graphcommand``
    Full path to ``gnuplot`` with optional command-line options.

``commentchars``
    Character(s) that denote the start of a comment.  Defaults to ``#%``.  The
    first character should not be changed without good reason.

``xtrapy``
    Path to a Python script to load at startup.  The path is resolved relative
    to the ``.inp`` file.  The script may define ``at_start()``,
    ``each_step()``, and/or ``at_end()`` functions.  See
    :doc:`python_scripting` for details.


``[time]``
~~~~~~~~~~

``steps``
    Integer value specifying the number of steps in the current simulation.
    This value must be smaller than or equal to the number of entries in the
    precipitation file.

``startpos``
    Position (line) in the precipitation input file used as start.  Counting
    starts at zero.

``starttime``
    Day at which the simulation should start.  If not specified the simulation
    starts at the first step in the precipitation input file.  If both
    ``starttime`` and ``startpos`` are specified, ``starttime`` is used.

``firststep``
    Duration to assume for the first time-step when no prior step is known.
    Default is 1.0 day.


``[run]``
~~~~~~~~~

``outputfile``
    Filename to save output to.  Can be overridden with the ``-o`` command-line
    option.

``runid``
    Not currently used.


``[determine]``
~~~~~~~~~~~~~~~

``canopy``
    Determine canopy water balance (see ``[canopy]``).

``evaporation``
    Determine actual evaporation (see ``[evaporation]``).

``pevaporation``
    Determine potential evaporation (see ``[pevaporation]``).

``soilmoisture``
    Determine soil moisture profile (see ``[soil]``).

``fit``
    Use nonlinear regression to fit to measured data (see ``[fit]``).


``[pevaporation]``
~~~~~~~~~~~~~~~~~~

``method``
    * 0 — potential evaporation via Penman :math:`E_0` (needs: ``refrad``,
      ``netrad``, ``rhumid``, ``windspeed``, ``temp``, ``inrad``)
    * 1 — potential evaporation via Penman :math:`E_0` using sun-ratio (needs:
      ``sunratio``, ``rhumid``, ``windspeed``, ``temp``, ``inrad``)
    * 2, 3 — not yet implemented
    * 4 — potential evaporation using Makkink (needs: ``rhumid``,
      ``windspeed``, ``temp``, ``inrad``)


``[evaporation]``
~~~~~~~~~~~~~~~~~

``method``
    * 0 — evaporation equal to potential evaporation
    * 1 — multiply potential evaporation by a crop factor (needs ``cropfac``)
    * 2 — calculate actual evaporation using the Penman–Monteith formula

``cropfac``
    Floating point crop factor by which potential evaporation is multiplied to
    yield actual evaporation (only needed for method = 1).


``[interception]``
~~~~~~~~~~~~~~~~~~

``method``
    * 0 — Gash analytical model
    * 1 — Rutter numerical model
    * 2 — LAI fraction
    * 3 — Calder regression

``gamma``
    Maximum interception loss per day :math:`\gamma` in the Calder equation [mm].

``delta``
    Fitting parameter :math:`\delta` in the Calder equation.

``E_avg/R``
    Average evaporation / average rainfall during a storm (Gash model).  If
    not set, Penman–Monteith with :math:`R_a = 0` is used.

``p_tr``
    Fraction of water diverted to the trunk (Gash, Rutter).

``p_f``
    Free throughfall coefficient (Gash, Rutter).

``S``
    Canopy storage in cm (Gash, Rutter).

``gashm``
    0 or 1.  If set to 1, an adapted Gash model is used that works for
    time-steps smaller than 1 day.  Default = 0.

``laifrac``
    Canopy interception coefficient (LAI fraction method).

``lai``
    Leaf area index.  Needed for the laifrac method.  If not present it is
    searched in the ``[canopy]`` section.


``[canopy]``
~~~~~~~~~~~~

``layers``
    Number of canopy layers.  At present only one layer is allowed.

``Rnet_absorb``
    Fraction of total radiation absorbed by the canopy
    (:math:`0 \le \text{Rnet\_absorb} \le 1`).  The remaining amount is used
    for soil evaporation.

``transpiration``
    Which transpiration equation to use.
    * 2 — Penman–Monteith
    * 3 — read from ``ptr`` in the ``[ts]`` section

``z``
    Height of the canopy [m].

``z_0``
    Aerodynamic roughness length [m].

``d``
    Zero plane displacement length [m].

``rs``
    Canopy resistance [s/m].  If not specified the user-defined regression
    function ``estrs()`` is used.

``drytime``
    If set, this value [days] determines how long it takes for the canopy to
    dry.

``wetevap``
    If set, this value [cm/day] determines the wet-canopy evaporation rate
    instead of Penman–Monteith with :math:`R_s = 0`.


``[roots]``
~~~~~~~~~~~

``depth``
    Depth of the root zone [cm].  To vary rooting depth in time use the
    ``drootz`` variable in the ``[ts]`` section.

``swsink``
    * 0 — sink term according to Feddes (needs: ``hlim1``, ``hlim2u``,
      ``hlim2l``, ``hlim3h``, ``hlim3l``, ``hlim4``)
    * 1 — sink term according to Hoogland (needs: ``hlim1``, ``hlim2u``,
      ``hlim2l``, ``hlim3``, ``hlim4``)

``swhypr``
    * 0 — linear relation between the points ``hlim3`` and ``hlim4``
    * 1 — hyperbolic relation between the points ``hlim3`` and ``hlim4``

``swupfu``
    * 0 — water uptake function according to Feddes
    * 1 — water uptake function according to Hoogland
    * 2 — water uptake function according to Prasad (1988)

``cofsza``
    Intercept *a* in Feddes et al. 1988 (only needed if ``swupfu`` = 1).

``cofszb``
    Slope *b* in Feddes et al. 1988 (only needed if ``swupfu`` = 1).

``hlim1``
    Pressure head [cm] below which roots start to extract water from the upper
    soil layer (starting point).

``hlim2u``
    Pressure head [cm] below which roots start to extract water optimally from
    the upper soil layer.

``hlim2l``
    As above, but for all lower soil layers.

``hlim3h``
    Pressure head [cm] below which roots cannot extract water optimally for a
    high potential transpiration rate (0.5 cm/d) — limiting point.

``hlim3l``
    As above, but for low potential transpiration rate (0.1 cm/d).

``hlim3``
    Pressure head [cm] below which roots cannot extract water any more
    (limiting point; Hoogland method).

``hlim4``
    Pressure head [cm] below which no water uptake by roots is possible
    (wilting point).


``[ts]``
~~~~~~~~

Lists the forcing time series.  Each variable name maps to a file path.  The
precipitation variable (``pre``) is mandatory.

File format: two whitespace-separated columns (time, value).  VAMPS assumes
column 0 = time and column 1 = value.  You can override this by appending
``,xcol,ycol`` to the filename (a comma is therefore not allowed in filenames).

Standard variables:

``pre``
    Precipitation [cm/day].  Also determines the simulation time-steps.

``rlh``
    Relative humidity [%].

``tem``
    Dry-bulb temperature [°C].

``win``
    Wind speed [m/s].

``nra``
    Net radiation [W/m²].

``ira``
    Incoming (global) radiation [W/m²].

``ptr``
    Potential transpiration [cm/day] (if pre-computed).

``spe``
    Potential soil evaporation [cm/day] (if pre-computed).

``inr``
    Interception [cm/day] (if pre-computed).

``hea``
    Head at the bottom node [cm] (bottom condition 4).

``rdp``
    Rooting depth [cm].  You must specify at least three points; other points
    are interpolated using a spline.

``gwt``
    Groundwater table [cm] (bottom condition 0).

``lai``
    Leaf area index.

``sca``
    Canopy storage [cm].


``[soil]``
~~~~~~~~~~

``outskip``
    Skip this many timesteps in soil output.  Used to reduce output file size.

``initprof``
    * 0 — water content profile (needs ``theta_initial`` array)
    * 1 — pressure head profile (needs ``h_initial`` array)
    * 2 — calculate pressure head profile (needs ``gw_initial``)

``gw_initial``
    Initial groundwater level [cm below field level].  Needed if
    ``initprof`` = 2.

``swredu``
    Reduction of soil evaporation:
    * 0 — no reduction
    * 1 — Black (1969) model
    * 2 — Boesten (1986) model
    * 3 — adapted Boesten (1986) model (takes into account actual surface moisture)

``cofred``
    Factor *alfa* in Black or *beta* in Boesten.  Not needed for
    ``swredu`` = 0.

``bottom``
    Bottom boundary condition (see :doc:`../model/soil` for descriptions):
    0 = groundwater table, 1 = fixed flux, 2 = seepage/infiltration,
    3 = flux as function of head, 4 = fixed head, 5 = zero flux, 6 = free
    drainage.

``smooth``
    Size of the running average used for smoothing the ``ksat``,
    ``theta_saturation``, and ``residual_water`` profiles.  Set to zero for no
    smoothing (default).

``layers``
    Number of computational soil layers.  The number of physical layers is
    determined by the number of ``[layer_n]`` sections.

``pondmx``
    Maximum amount of ponding [cm] at the top of the profile.  Defaults to 0.0.

``gwlevel``
    Water level at the bottom of the profile [cm].  Needed if ``bottom`` = 4.

``dtmax``
    Maximum timestep [days] in the soil module.  Lower this if you experience
    large mass-balance errors.

``dtmin``
    Minimum timestep [days] in the soil module.  Setting ``dtmin`` and
    ``dtmax`` to equal values forces VAMPS to use a fixed timestep.

``speed``
    Integer from 1 (slow/accurate) to 6 (fastest/least accurate) controlling
    the trade-off between calculation accuracy and speed.  Combines the
    settings of ``dtmin``, ``thetol``, ``solvemet``, ``mktable``, ``maxitr``,
    and ``swnums``.  Individually specified variables override the speed
    preset.  Default is 3.

``maxitr``
    Maximum number of iterations in the soil module.  Iterations are only
    performed if ``swnums`` != 1.

``thetol``
    Theta tolerance.  VAMPS checks if the solution is good enough and
    iterates if needed.  Usually between 1.0E-2 and 1.0E-5.  The default
    value usually works fine.

``solvemet``
    How VAMPS solves the equation matrix:
    * 0 — default tridiagonal (Thomas algorithm)
    * 1 — always use band-diagonal
    * 2 — very general solution (regains full machine precision; slow)

``swnums``
    If set to 1 the soil module will not check for convergence; it assumes
    the initial maximum ``dt`` is a good guess.  Much faster but can give
    poor results.

``mktable``
    If set to TRUE VAMPS creates lookup tables for the :math:`\theta`–:math:`K`
    relation and uses them during iteration.  Default is FALSE.  Can speed up
    calculations by up to 50 % at a slight loss of precision.  The default
    table size is 300 points; change with ``tablesize``.

``tablesize``
    Size of the lookup tables.  Default 300.  Increasing improves accuracy at
    some memory and performance cost.  Table sizes up to 1200 still provide
    speed improvements over not using tables.

``dumptables``
    If TRUE the lookup tables are dumped to the initial section of the output
    file (useful for plotting pF curves).

``estdmc``
    If TRUE and ``mktable`` is also TRUE, the dmc table is built using
    ``ts_slopes`` and the pF curve instead of the ``h2dmc`` function.

``verbose``
    Verbose level in the soil module (0 = silent, 1 = show estimated run time).

``smddepth``
    If set, the SMD (soil moisture deficit) is calculated to this depth.
    Otherwise the rooting depth is used.

``fieldcap``
    Head [cm] at which the soil is at field capacity.  Needed for SMD
    calculation.  Default = −100.0.

``mbck``
    If set to 1, VAMPS uses a simple mass balance check for convergence
    instead of ``thetol``.  Usually faster for the same accuracy.

``mbalerr``
    Required mass balance accuracy per timestep when ``mbck`` = 1.  A value
    between 1.0E-2 and 1.0E-5 is usually best.  Default = 0.5E-3.


``[drainage]``
~~~~~~~~~~~~~~

This section is experimental.

``method``
    * 0 — no lateral drainage (default)
    * 1 — TOPOG type drainage (only at saturation)
    * 2 — allow unsaturated lateral flow as well

    If ``method`` > 0 then ``slope`` must also be set.

``slope``
    Slope used in the calculation of lateral drainage.

``exclude``
    Array of layer indices in which lateral drainage is not allowed.
    Example: ``exclude = 12 1 23 45``.  Combined with a no-flow bottom
    boundary this can simulate a lysimeter.


``[soilsectionname]``
~~~~~~~~~~~~~~~~~~~~~~

This section can have *any* name and contains the soil-type-specific
parameters.  The ``soilsection`` variable in each ``[layer_n]`` section
references this section by name.

``method``
    Method for the :math:`K` vs :math:`\theta` relation:

    .. list-table::
       :header-rows: 1
       :widths: 10 90

       * - Value
         - Description
       * - 0
         - Clapp/Hornberger
       * - 1
         - Van Genuchten (default)
       * - 2
         - Not yet implemented
       * - 3
         - Van Genuchten parameters determined from :math:`\theta` vs pF pairs
       * - 4
         - Read TOPOG_soil lookup tables
       * - 5
         - User-defined Python functions (requires Python support; see :doc:`python_scripting`)
       * - 6
         - Brooks and Corey (1964)

    For method 3: given values of ``alpha`` and ``n`` are used as initial
    guesses.  Required variables: ``theta`` (array) and ``pf`` (array).  The
    exponent ``l`` is fixed at 0.5.

    For method 4: VAMPS reads and uses soil tables generated by the TOPOG_soil
    programme.

    For method 5: define ``getspars``, ``h2t``, ``t2k``, ``t2h``, ``h2dmc``,
    ``h2k``, ``h2u``, and ``h2dkdp`` Python functions.  See
    :doc:`python_scripting` and ``share/soilf.py`` / ``share/soilf_bc.py``
    for reference implementations.  It is strongly recommended to also set
    ``mktable = 1`` in ``[soil]`` for acceptable performance.

    For method 6: required parameters are ``lambda``, ``hb``, ``thetas``,
    ``theta_residual``, ``ksat``.

``description``
    Optional description of the soil layer.

``ksat``
    Saturated hydraulic conductivity of the layer.

``kh/kv``
    Ratio of :math:`K_{sat}` horizontal to :math:`K_{sat}` vertical.  Only
    used with lateral drainage.  Default = 1.

``thetas``
    Water content at saturation (porosity).

``psisat``
    Head at saturation (air entry value) — needed for Clapp/Hornberger
    (method = 0).

``b``
    Factor *b* in Clapp/Hornberger (method = 0).

``theta_residual``
    Residual soil moisture content.

``alpha``
    :math:`\alpha` parameter in Van Genuchten [1/cm].

``l``
    :math:`l` parameter in Van Genuchten (use 0.5 if not measured).

``n``
    :math:`n` parameter in Van Genuchten.

``lambda``
    Pore-size distribution index :math:`\lambda` for Brooks and Corey
    (method = 6).  Starting approximation from Van Genuchten:
    :math:`\lambda \approx n - 1`.

``hb``
    Air-entry (bubbling) pressure head :math:`h_b` for Brooks and Corey
    (method = 6), in cm.  Must be negative (e.g. ``hb = -16.4``).
    Starting approximation from Van Genuchten: :math:`h_b \approx -1/\alpha`.

``tablefile``
    File from which the soil table is read.  Use with method = 4.

``tablefiletype``
    Format of the table file:

    * 1 — TOPOG format (VAMPS uses columns 1, 3, 4, and 5)
    * 2 — whitespace-separated columns (psi theta k); differential moisture
      capacity is estimated
    * 3 — whitespace-separated columns (psi theta k diff_moist)

    All table files should be made with *descending* theta values.


``[layer_n]``
~~~~~~~~~~~~~

Only ``[layer_0]`` is mandatory.  Additional sections are needed only if
you have more than one physical soil layer.

``thickness``
    Thickness of the layer [cm].

``soilsection``
    Name of the section that contains the soil-type parameters for this layer.


``[fit]``
~~~~~~~~~

Used with the ``--fit`` option or when ``fit = 1`` in ``[determine]``.

``fitto``
    Name of a time series (must be in the ``[ts]`` section) with actual
    measurements to fit against.

``n`` (where n = 0 … MAXPAR)
    Add a parameter to the fitting list.  Available parameters:
    1 = n (VG), 2 = alpha, 3 = ksat, 4 = b, 5 = psisat, 6 = l,
    7 = residual_water, 8 = thetas.

``layer``
    Physical layer to use in fitting.
