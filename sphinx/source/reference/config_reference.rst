Configuration File Reference — vamps(5)
=======================================

This page is the authoritative reference for the VAMPS input file format.
It is derived from the ``vamps.5`` man page.

For tutorial-style documentation see :doc:`../user_guide/configuration`.


File format
-----------

The input file format is similar to Windows ``.ini`` files.  All names and
sections are case-insensitive.  String variable values preserve case, so
filenames are case-sensitive if the operating system is.

.. code-block:: ini

   [vamps]
   verbose = TRUE
   # This is a comment

   [environment]
   caseid = Interception test file. Bisley catchment.\
   Summer of 1995

The ``\`` character breaks long strings over more than one line.  Only the
first ``=`` sign is significant.  Spaces within variable names are preserved
verbatim.

Variable types: *floats*, *arrays* (whitespace-separated floats), *strings*,
*integers*, *characters*, *boolean* (0/1, FALSE/TRUE, NO/YES).


Allowed set names (``--Output``)
---------------------------------

``pre``
    Precipitation [cm/day]
``rlh``
    Relative humidity [%]
``hea``
    Head at bottom [cm]
``rdp``
    Rooting depth [cm]
``tem``
    Dry-bulb temperature [°C]
``gwt``
    Groundwater table [cm]
``inr``
    Interception [cm/day]
``trf``
    Throughfall [cm]
``stf``
    Stemflow [cm]
``pev``
    Potential evaporation [cm/day]
``spe``
    Potential soil evaporation [cm/day]
``ptr``
    Potential transpiration [cm/day]
``qbo``
    Flow through bottom of profile [cm/day]
``vol``
    Actual water content [cm]
``avt``
    Average theta in profile
``lai``
    Leaf area index
``sca``
    Canopy storage [cm]
``ira``
    Incoming radiation [W/m²]
``nra``
    Net radiation [W/m²]
``ref``
    Reflected radiation [W/m²]
``win``
    Wind speed [m/s]
``sur``
    Sun-ratio [n/N]


``[time]``
----------

``steps``
    Integer number of steps in the current simulation.  Must be ≤ the number
    of entries in the precipitation file.

``starttime``
    Day at which the simulation should start.  If not specified, simulation
    starts at the first step in the precipitation file.  If both
    ``starttime`` and ``startpos`` are specified, ``starttime`` takes
    precedence.

``startpos``
    Position (line number, counting from 0) in the precipitation input file
    used as the start.

``firststep``
    Duration [days] assumed for the first timestep.  Default 1.0.


``[run]``
---------

``outputfile``
    Filename for output.  Overridable with the ``-o`` command-line option.

``runid``
    Not currently used.

``description``
    Optional string description.


``[xout]``
----------

``filename``
    Filename for extra output in column-type format.  No extra output is
    generated if this variable is absent.


``[determine]``
---------------

``canopy``
    Determine canopy water balance.

``evaporation``
    Determine actual evaporation.

``pevaporation``
    Determine potential evaporation.

``soilmoisture``
    Determine soil-moisture profile.


``[pevaporation]``
------------------

``method``
    * 0 — Penman :math:`E_0` (needs: ``refrad``, ``netrad``, ``rhumid``, ``windspeed``, ``temp``, ``inrad``)
    * 1 — Penman :math:`E_0` with sun-ratio (needs: ``sunratio``, ``rhumid``, ``windspeed``, ``temp``, ``inrad``)
    * 4 — Makkink (needs: ``rhumid``, ``windspeed``, ``temp``, ``inrad``)


``[evaporation]``
-----------------

``method``
    * 0 — evaporation = potential evaporation
    * 1 — multiply potential evaporation by crop factor (needs ``cropfac``)
    * 2 — Penman–Monteith formula

``cropfac``
    Crop factor for method 1.


``[interception]``
------------------

``method``
    * 0 — Gash
    * 1 — Rutter
    * 2 — LAI fraction
    * 3 — Calder

``gamma``
    Maximum interception loss per day :math:`\gamma` [mm] (Calder).

``delta``
    Fitting parameter :math:`\delta` (Calder).

``E_avg/R``
    Average evaporation / average rainfall during a storm (Gash).  If not
    set, Penman–Monteith with :math:`R_a = 0` is used.

``p_tr``
    Fraction of water diverted to trunk (Gash, Rutter).

``p_f``
    Free throughfall coefficient (Gash, Rutter).

``S``
    Canopy storage [cm] (Gash, Rutter).

``gashm``
    0 or 1.  If 1, adapted Gash model for sub-daily time-steps.  Default 0.

``laifrac``
    Canopy interception coefficient (LAI fraction method).

``lai``
    Leaf area index.  Needed for the laifrac method.


``[top]``
---------

``system``
    Integer specifying the top-system module:

    * 0 — empty (not implemented)
    * 1 — bare soil
    * 2 — full canopy (not yet implemented)
    * 3 — partial canopy (not yet implemented)
    * 4 — all canopy fluxes pre-computed
    * 5 — old canopy.c (Penman–Monteith + Gash interception)
    * 6 — Python-scripted top-system (requires ``xtrapy``)

``soilevaporation``
    Method for soil evaporation when using the notree top-system (system = 1):
    0 = E0SUNRAD, 1 = E0NETRAD, 2 = PENMON_NOSOIL, 3 = PENMON_SOIL,
    4 = MAKKINK.  Always needs ``rlh``, ``tem``; additional requirements
    depend on the method.


``[canopy]``
------------

``layers``
    Number of canopy layers.  Only one layer is currently supported.

``Rnet_absorb``
    Fraction of total radiation absorbed by the canopy (0 < value < 1).

``transpiration``
    * 2 — Penman–Monteith
    * 3 — read from ``ptr`` in ``[ts]``

``z``
    Height of the canopy [m].

``z_0``
    Aerodynamic roughness length [m].

``d``
    Zero plane displacement length [m].

``rs``
    Canopy resistance [s/m].  If absent, ``estrs()`` is used.

``drytime``
    Time [days] for the canopy to dry.

``wetevap``
    Wet-canopy evaporation rate [cm/day].


``[roots]``
-----------

``depth``
    Root zone depth [cm].

``swsink``
    Sink term method: 0 = Feddes, 1 = Hoogland.

``swhypr``
    0 = linear, 1 = hyperbolic relation between ``hlim3`` and ``hlim4``.

``swupfu``
    Water uptake function: 0 = Feddes, 1 = Hoogland, 2 = Prasad (1988),
    3 = simple (no reduction).

``cofsza``
    Intercept *a* (Feddes, only if ``swupfu`` = 1).

``cofszb``
    Slope *b* (Feddes, only if ``swupfu`` = 1).

``hlim1``
    Pressure head [cm] — starting point for root extraction.

``hlim2u``
    Pressure head [cm] — optimal extraction starts (upper layer).

``hlim2l``
    Pressure head [cm] — optimal extraction starts (lower layers).

``hlim3h``
    Pressure head [cm] — limiting point for high transpiration rate (0.5 cm/d).

``hlim3l``
    Pressure head [cm] — limiting point for low transpiration rate (0.1 cm/d).

``hlim3``
    Pressure head [cm] — limiting point (Hoogland method).

``hlim4``
    Pressure head [cm] — wilting point; no root uptake below this.


``[ts]``
--------

Must contain at least ``pre`` (precipitation).

``pre``
    Precipitation [cm].  Also determines simulation time-steps.

``pev``
    Potential evaporation [cm].

``ptr``
    Potential transpiration [cm].

``spe``
    Potential soil evaporation [cm].

``inr``
    Interception [cm].

``rdp``
    Rooting depth [cm] (spline-interpolated, needs ≥ 3 points).

``qbo``
    Flux at bottom node [cm] (bottom condition 1).

``hea``
    Head at bottom node [cm] (bottom condition 4).

``gwt``
    Groundwater level [cm] (bottom condition 0).

``rlh``
    Relative humidity [%].

``tem``
    Temperature [°C].

``win``
    Wind speed [m/s].

``nra``
    Net radiation [W/m²].

``ira``
    Incoming radiation [W/m²].

``lai``
    Leaf area index.

``sca``
    Canopy storage [cm].


``[soil]``
----------

``outskip``
    Skip every *n* timesteps in output.

``bottom``
    Bottom boundary condition:
    0 = groundwater table, 1 = given flux, 2 = seepage/infiltration,
    3 = flux as function of head, 4 = given pressure head, 5 = zero flux,
    6 = free drainage.

``initprof``
    * 0 — water content profile (needs ``theta_initial``)
    * 1 — pressure head profile (needs ``h_initial``)
    * 2 — calculate from groundwater level (needs ``gw_initial``)

``gw_initial``
    Initial groundwater level [cm below field level].

``swredu``
    Soil evaporation reduction: 0 = none, 1 = Black, 2 = Boesten,
    3 = adapted Boesten.

``cofred``
    Factor *alfa* (Black) or *beta* (Boesten).

``smooth``
    Running-average window for smoothing ksat and theta profiles.

``layers``
    Number of computational soil layers.

``pondmx``
    Maximum ponding [cm].  Default 0.0.

``gwlevel``
    Water level at bottom [cm].  Needed for ``bottom`` = 4.

``speed``
    1 (slow) to 6 (fast) speed preset.  Default 3.

``dtmax``
    Maximum internal timestep [days].

``dtmin``
    Minimum internal timestep [days].

``tm_mult``
    Multiplier for dt estimation.  Default 3.

``maxitr``
    Maximum number of iterations.

``thetol``
    Theta tolerance for convergence.  Typical range: 1.0E-2 to 1.0E-5.

``mbck``
    If 1, use mass-balance check for convergence instead of ``thetol``.

``mbalerr``
    Required mass-balance accuracy when ``mbck`` = 1.  Default 0.5E-3.

``solvemet``
    Matrix solver: 0 = tridiagonal, 1 = band-diagonal, 2 = general.

``noit``
    If 1, skip convergence check (fast but potentially inaccurate).

``mktable``
    If TRUE, create lookup tables for :math:`\theta`–:math:`K`.

``tablesize``
    Lookup table size.  Default 300.

``dumptables``
    If TRUE, dump lookup tables to the output file initial section.

``estdmc``
    If TRUE and ``mktable`` TRUE, build dmc table from ts_slopes.

``verbose``
    Verbose level (0 = silent).

``smddepth``
    Depth [cm] for SMD calculation.

``fieldcap``
    Field capacity head [cm].  Default −100.0.


``[drainage]``
--------------

``method``
    0 = none, 1 = TOPOG-type (saturated), 2 = also unsaturated.

``slope``
    Slope for lateral drainage calculation.

``exclude``
    Array of layer indices excluded from lateral drainage.


``[soilsectionname]``
---------------------

This section can have any name.  Referenced from ``[layer_n]`` via
``soilsection``.

``method``
    Soil hydraulic model:

    .. list-table::
       :header-rows: 1
       :widths: 10 90

       * - Value
         - Description
       * - 0
         - Clapp/Hornberger — parameters: ``b``, ``psisat``, ``thetas``, ``ksat``
       * - 1
         - Van Genuchten — parameters: ``alpha``, ``n``, ``l``, ``thetas``, ``theta_residual``, ``ksat``
       * - 2
         - Not yet implemented
       * - 3
         - Van Genuchten fit from (theta, pF) pairs
       * - 4
         - TOPOG_soil lookup table (``tablefile``, ``tablefiletype``)
       * - 5
         - User-defined Python functions (``xtrapy``)
       * - 6
         - Brooks and Corey (1964) — parameters: ``lambda``, ``hb``, ``thetas``, ``theta_residual``, ``ksat``

``description``
    Optional layer description.

``ksat``
    Saturated hydraulic conductivity.

``kh/kv``
    Anisotropy ratio (for lateral drainage).

``thetas``
    Water content at saturation.

``psisat``
    Air-entry pressure head [cm] — Clapp/Hornberger.

``b``
    Parameter *b* — Clapp/Hornberger.

``theta_residual``
    Residual water content.

``alpha``
    :math:`\alpha` [1/cm] — Van Genuchten.

``l``
    :math:`l` — Van Genuchten (default 0.5).

``n``
    :math:`n` — Van Genuchten.

``lambda``
    Pore-size distribution index :math:`\lambda` — Brooks and Corey (method 6).
    Approximation from Van Genuchten: :math:`\lambda \approx n - 1`.

``hb``
    Air-entry (bubbling) pressure head [cm, must be negative] —
    Brooks and Corey (method 6).
    Approximation from Van Genuchten: :math:`h_b \approx -1/\alpha`.
    Example: ``hb = -16.4``.

``tablefile``
    Soil table file path (method 4).

``tablefiletype``
    1 = TOPOG format, 2 = (psi theta k), 3 = (psi theta k dmc).
    All tables must have *descending* theta values.


``[layer_n]``
-------------

Only ``[layer_0]`` is mandatory.

``thickness``
    Layer thickness [cm].

``soilsection``
    Name of the soil-type section for this layer.


Output file sections
--------------------

``[header]``
~~~~~~~~~~~~

``run_start_time``
    Time at which the run started.

``command``
    The command that produced the file.

``defaultsfile``
    Name of the defaults file used.

``infilename``
    Name of the VAMPS input file.


``[initial]``
~~~~~~~~~~~~~

``steps``
    Number of timesteps.

``layers``
    Number of soil layers.

``volini``
    Initial water content of profile [cm].

``volsat``
    Saturated water content of profile [cm].

``z``
    Array of layer depths [cm].

``theta``
    Initial water content per layer.

``h``
    Initial head per layer.

``k``
    Initial unsaturated conductivity per layer.

``as_above``
    Whether a layer inherited settings from the layer above.


``[t_n]`` (per-timestep output)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``t``
    Actual date at timestep *n*.

``interception``
    Canopy interception [cm].

``transpiration``
    Potential transpiration [cm].

``throughfall``
    Throughfall [cm].

``stemflow``
    Stemflow [cm].

``dt``
    Last sub-timestep size.

``avgtheta``
    Average theta over the whole profile.

``SMD``
    Soil moisture deficit.

``pond``
    Ponding [cm].

``surface_runoff``
    Cumulative surface runoff [cm].

``cumeva``
    Cumulative soil evaporation [cm].

``cumtra``
    Cumulative transpiration [cm].

``cumprec``
    Cumulative precipitation [cm].

``cumintc``
    Cumulative interception [cm].

``cqbot``
    Cumulative flux through the bottom [cm] (from pore volume change;
    not valid if profile is saturated).

``cumtop``
    Cumulative flux through the top [cm].

``qtop``
    Flux through the top this timestep [cm/d].

``qbot``
    Flux through the bottom boundary this timestep [cm/d].  Positive values
    indicate downward flow out of the profile.

``cumbot``
    Cumulative flux through the bottom boundary [cm].

``cqbotts``
    Cumulative flux through the bottom within the current daily timestep [cm].
    Reset to zero at the start of each timestep; useful for diagnosing
    sub-daily bottom outflow.

``runots``
    Cumulative surface runoff within the current daily timestep [cm].
    Reset to zero at the start of each timestep.

``soilevaporation``
    Actual (reduced) soil evaporation this timestep [cm/d].  May be less than
    ``pot_soilevaporation`` when the near-surface layers are dry (``swredu=1``).

``pot_soilevaporation``
    Potential soil evaporation this timestep [cm/d], as supplied by the
    top-system module.

``prec``
    Gross precipitation for this timestep [cm/d].

``intc``
    Canopy interception for this timestep [cm/d].  See also ``interception``.

``ptra``
    Potential transpiration for this timestep [cm/d], as supplied by the
    top-system module.

``rootextract``
    Cumulative actual root water extraction since the start of the run [cm].
    Derived from the integral of ``qrot`` over all sub-timesteps and layers.
    Used in the mass balance equation.

``rootts``
    Actual root water extraction rate for this daily timestep [cm/d], obtained
    by integrating ``qrot`` over all sub-timesteps within the day and dividing
    by the timestep length.  This is the true extracted transpiration; compare
    with ``ptra`` (potential) and ``cumtra`` (cumulative potential).

``masbal``
    Mass balance residual [cm] at the end of the timestep::

        masbal = volini + cumbot − cumdra − cumtop − rootextract − volact

    A non-zero value indicates a solver error.  Values smaller than ~0.01 cm
    are generally acceptable.

``volact``
    Actual water volume in the entire soil profile [cm] at the end of the
    timestep.

``theta``
    Array of volumetric water content per layer (length ``layers``).
    Also written in the ``[initial]`` section for the initial state.

``h``
    Array of pressure heads per layer [cm] (length ``layers``).
    Also written in the ``[initial]`` section.

``k``
    Array of unsaturated hydraulic conductivity per layer [cm/d]
    (length ``layers``).  Also written in the ``[initial]`` section.

``gwl``
    Array of length 2 containing the groundwater levels [cm].

``q``
    Array of fluxes at each node boundary [cm/d] (length ``layers + 1``).
    ``q[0]`` is the top boundary flux, ``q[layers]`` is the bottom boundary flux.

``inq``
    Array of cumulative inflow at each node boundary within the current
    timestep [cm] (length ``layers + 1``).

``qrot``
    Array of root water extraction rates per layer [cm/d] (length ``layers``).
    Each element is the total extraction from that layer in the current
    sub-timestep, not per unit depth.  Summing over all layers gives the
    instantaneous actual transpiration rate (after Feddes stress reduction).

``howsat``
    Array of relative saturation per layer (length ``layers``), defined as
    ``1 − (theta_s − theta)``.  A value of 1 means the layer is at saturation.

Drainage variables (only present when lateral drainage is active, ``dodrain > 0``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``cqdra``
    Cumulative lateral drainage for the current timestep [cm].

``cumdra``
    Total cumulative lateral drainage since the start of the run [cm].

``drainage``
    Array of lateral drainage rates per layer [cm/d] (length ``layers``).

Iteration diagnostics
^^^^^^^^^^^^^^^^^^^^^

``converror``
    Number of convergence failures for this timestep.

``itter``
    Average number of matrix solver iterations per sub-timestep.
