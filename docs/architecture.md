# VAMPS Architecture

VAMPS (VAriably saturated soil Model with Plant and canopy System) is a
1-D unsaturated-zone water-flow model.  The core physics — the Richards
equation solver with adaptive internal sub-stepping — is implemented in C.
All I/O, configuration, external timestep control, and post-processing live
in Python.

---

## 1. Layered overview

```mermaid
block-beta
  columns 1

  block:user["User / scripts / notebooks"]
    A["Model.from_file('run.inp')\nModel(config, forcing)\nresult = model.run_stepwise()"]
  end

  block:pyapi["vampspy  —  Python API layer"]
    B["model.py\nModel class"]
    C["forcing.py\nload_ts_spec / read_ts"]
    D["_io.py\nparse_inp / write_inp / read_out"]
  end

  block:ext["_vampscore.so  —  Python C extension"]
    E["run()\n(whole-run, backward compat)"]
    F["soil_init / soil_step / soil_state\n(stepwise API)"]
  end

  block:clayer["C core — soil physics"]
    G["soil_api.c\nvamps_init_stepwise\nvamps_do_step\nvamps_get_state"]
    H["vamps_ext.c\nvamps_run_ext"]
    I["vamps.c\nprelim / dorun / loaddefaults"]
  end

  block:csolvers["C solvers  (never touched by Python)"]
    J["soil/\nheadcalc · timestep · fluxes\nrootex · soilboun · integral"]
    K["topsys/\ncanopy · pre_can · notree"]
    L["support libs\nts.lib · deffile.lib · met.lib · nr_ut.lib"]
  end

  user --> pyapi
  pyapi --> ext
  ext --> clayer
  clayer --> csolvers
```

---

## 2. Three execution paths

The same `Model` class supports three routes to a result.  All three
produce bit-for-bit identical output.

```mermaid
flowchart LR
    subgraph Python
        M["Model(config, forcing)"]
        FF["Model.from_file(inp)"]
    end

    M & FF --> choice{{"_vampscore\navailable?"}}

    choice -- yes --> P3
    choice -- no  --> P1

    subgraph P1["Path 1 — subprocess (fallback)"]
        direction TB
        p1a["write forcing → .ts files"]
        p1b["write config  → .inp file"]
        p1c["spawn vamps binary"]
        p1d["parse .out file → numpy"]
        p1a --> p1b --> p1c --> p1d
    end

    subgraph P2["Path 2 — run()  (whole-run C loop)"]
        direction TB
        p2a["write config → temp .inp"]
        p2b["register forcing arrays\nts_register_array()"]
        p2c["_vampscore.run()\n→ vamps_run_ext()\n→ C drives outer loop"]
        p2d["return numpy dict"]
        p2a --> p2b --> p2c --> p2d
    end

    subgraph P3["Path 3 — run_stepwise()  (Python loop)"]
        direction TB
        p3a["write config → temp .inp"]
        p3b["register forcing arrays\nts_register_array()"]
        p3c["_vampscore.soil_init()"]
        p3d["for i in range(steps):\n  soil_step(i)\n  soil_state(i)"]
        p3e["assemble numpy dict"]
        p3a --> p3b --> p3c --> p3d --> p3e
    end

    choice -- yes / run_stepwise --> P3
    choice -- yes / run          --> P2
```

---

## 3. Timestep hierarchy

VAMPS has two nested levels of time iteration.  Only the outer one is
visible to Python.

```mermaid
sequenceDiagram
    participant Py  as Python<br/>(model.py)
    participant API as soil_api.c<br/>vamps_do_step()
    participant Top as topsys/<br/>tstep_top()
    participant Soi as soil/<br/>tstep_soil()
    participant Sub as soil/<br/>headcalc + timestep<br/>(internal sub-steps)

    loop External timestep  i = 0 … steps-1
        Py  ->> API : soil_step(i)
        API ->> Top : tstep_top(i, &pre, &inr, &ptr, &spe)
        Note over Top: Canopy water balance<br/>(interception, transp, soil-evap)
        Top -->> API: pre / inr / ptr / spe updated in-place

        API ->> Soi : tstep_soil(i, t, pre, inr, ptr, spe)

        loop Internal sub-steps  (adaptive dt)
            Soi ->> Sub : headcalc()
            Note over Sub: Solve Richards equation<br/>(tridiagonal / band / general)
            Sub -->> Soi: θ, h, k updated
            Soi ->> Sub : timestep() → new dt
            Note over Sub: dt = thetol · dz / |dθ/dt|<br/>clamped to [dtmin, dtmax]
        end

        Soi -->> API: t advanced to forcing time[i]
        API -->> Py : (returns)
        Py  ->> API : soil_state(i)  →  vamps_state_t
    end
```

---

## 4. Soil physics pipeline (one external step)

```mermaid
flowchart TD
    A["tstep_soil(i, t, pre, inr, ptr, spe)"]

    A --> B["presoil_tstep()\nSet top boundary condition\n(infiltration or ponding)"]

    B --> C{{"Saturated\nprofile?"}}
    C -- yes --> Csat["Use saturated dt\n(dtsat, larger step)"]
    C -- no  --> Cuns["Adaptive dt from θ-change rate\nthetol · dz / |dθ/dt|"]
    Csat & Cuns --> D

    D["headcalc()\nSolve discretised Richards equation\n∂θ/∂t = ∂/∂z[K(θ)(∂h/∂z + 1)] − S(z)"]

    D --> E{{"Solver\nmethod?"}}
    E -- "TRI (homogeneous)" --> E1["Tridiagonal solver\n(Thomas algorithm)"]
    E -- "BAN (heterogeneous)" --> E2["Band-diagonal solver\n(bandec / banks)"]
    E -- "GEN (complex)" --> E3["General matrix solver"]

    E1 & E2 & E3 --> F["rootex()\nRoot water uptake S(z)\n(Feddes stress functions)"]

    F --> G["fluxes()\nCompute inter-layer fluxes q[i]"]

    G --> H["soilboun()\nApply boundary conditions\ntop: infiltration / runoff\nbottom: GWT / fixed head / free drainage"]

    H --> I["integral()\nCompute volact, SMD, avgtheta\nfrom θ profile"]

    I --> J{{"Converged?\n‖θ − θ_prev‖ < tol"}}
    J -- no --> D
    J -- yes --> K["postsoil_tstep()\nUpdate cumulative totals\ncumprec / cumtra / cumeva / cumintc / masbal"]
```

---

## 5. Python ↔ C data boundary

What crosses the `_vampscore` extension interface, and what stays inside C:

```mermaid
flowchart LR
    subgraph Python
        direction TB
        CFG["config dict\n(INI sections as nested dicts)"]
        FORCE["forcing dict\n{name: np.ndarray}"]
        RESULT["result dict\nscalars + profiles as np.ndarray"]
    end

    subgraph Extension["_vampscore.so  (Python C extension)"]
        direction TB
        REG["ts_register_array()\nfrom numpy → C XY array"]
        INIT["vamps_init_stepwise()\n→ prelim()  reads config file"]
        STEP["vamps_do_step()\n→ tstep_top + tstep_soil"]
        STATE["vamps_get_state()\nvamps_get_theta()\nvamps_get_profiles()\nC globals → Python dict"]
    end

    subgraph C_globals["C global state  (never Python objects)"]
        direction TB
        G1["Scalars\nvolact · SMD · qtop · qbot\navgtheta · masbal\nt · cumprec · cumtra · cumeva · cumintc"]
        G2["Profiles  length layers\ntheta · k · h · qrot · howsat"]
        G3["Profiles  length layers+1\nq · inq"]
        G4["gwl[2]\n(groundwater levels)"]
    end

    CFG  -- "write temp .inp\n(Python → file → C)" --> INIT
    FORCE -- "double* arrays\n(zero-copy view)" --> REG
    REG --> INIT
    INIT --> STEP
    STEP <--> C_globals
    C_globals --> STATE
    STATE -- "copy to numpy\n(C → Python)" --> RESULT
```

### Result dict layout

| Key | Shape | Description |
|-----|-------|-------------|
| `t`, `volact`, `SMD`, `qtop`, `qbot`, `avgtheta` | `(steps,)` | Scalar time-series |
| `cumprec`, `cumtra`, `cumeva`, `cumintc`, `masbal` | `(steps,)` | Cumulative totals |
| `precipitation`, `interception`, `transpiration`, `soilevaporation` | `(steps,)` | Per-step canopy fluxes |
| `theta` | `(steps, nlayers)` | Volumetric water content |
| `k` | `(steps, nlayers)` | Hydraulic conductivity |
| `h` | `(steps, nlayers)` | Pressure head |
| `qrot` | `(steps, nlayers)` | Root water uptake |
| `howsat` | `(steps, nlayers)` | Degree of saturation |
| `q` | `(steps, nlayers+1)` | Inter-layer flux (includes top/bottom boundaries) |
| `inq` | `(steps, nlayers+1)` | Cumulative inter-layer flux |
| `gwl` | `(steps, 2)` | Groundwater table levels |
| `_steps`, `_nlayers` | int | Metadata |

---

## 6. Canopy top-system types

The topsys module selects a canopy implementation at initialisation via
`init_top(system)`.  The integer constant in the `[top]` config section
determines which is used.

```mermaid
flowchart TD
    TC["init_top(system)"]

    TC --> T0["0 · TOP_NOOP\nAborts — not implemented"]
    TC --> T1["1 · TOP_SOIL\nBare soil\nno interception or transpiration"]
    TC --> T2["2 · TOP_FUL_CANOP\nFull canopy — not yet implemented"]
    TC --> T3["3 · TOP_PAR_CANOP\nPartial canopy — not yet implemented"]
    TC --> T4["4 · TOP_SCRIPT\nPython-scripted canopy\n(requires HAVE_LIBPYTHON)"]
    TC --> T5["5 · TOP_PRE_CANOP  ★ default\nAll fluxes pre-computed\nPass-through from forcing arrays\n(pre, inr, ptr, spe)"]
    TC --> T6["6 · TOP_OCANOP\nOld canopy.c (v0.99b)\nPenman–Monteith ET\nGash interception model"]

    style T5 stroke:#2a6,stroke-width:2px
    style T6 stroke:#26a,stroke-width:2px
```

---

## 7. Python module dependency map

```mermaid
graph TD
    init["vampspy/__init__.py\nexports: Model, forcing"]

    init --> model["vampspy/model.py\nModel\n  .from_file()\n  .run()\n  .run_stepwise()\n  ._run_core()\n  ._run_subprocess()"]

    init --> forcing["vampspy/forcing.py\nread_ts()\nread_ts_timed()\nload_ts_spec()\nload_forcing_dir()"]

    model --> io["vampspy/_io.py\nparse_inp()\nparse_out()\nwrite_inp()\nwrite_ts()\nread_out()"]

    model --> forcing
    model --> core["vampspy/_vampscore.so\nrun()\nsoil_init()\nsoil_step()\nsoil_state()\nsoil_nlayers()"]

    core --> capi["src/main/soil_api.c\nvamps_init_stepwise()\nvamps_do_step()\nvamps_get_state()\nvamps_get_theta()\nvamps_get_profiles()\nvamps_nlayers()"]

    core --> ext["src/main/vamps_ext.c\nvamps_run_ext()"]

    capi & ext --> prelim["src/main/vamps.c\nprelim()\nloaddefaults()"]

    prelim --> soil["src/soil/\nheadcalc · timestep · fluxes\nrootex · soilboun · integral\ngetparm · alloc · filltab …"]

    prelim --> topsys["src/topsys/\nintopsys · canopy · pre_can\nnotree · topout"]

    prelim --> deffile["src/deffile.lib/\ngetdefint · getdefstr\nrinmem · setvar"]

    prelim --> tslib["src/ts.lib/\nget_data · ts_readf\nts_register_array"]

    topsys --> metlib["src/met.lib/\npenmon · ra · e0 · vslope …"]

    soil --> nrlib["src/nr_ut.lib/\nnrutil · nr_mat …"]
```

---

## 8. State reset between runs

When starting a new run (whether via `run()` or `run_stepwise()`), several
C globals must be reset to avoid contamination from a previous run.

```mermaid
flowchart LR
    Start(["New run\nvamps_init_stepwise()"])

    Start --> A["del_all_sets()\nFree all ts dataset arrays"]
    Start --> B["reset_presoil()\nClear firsttime flag\nso presoil() re-initialises"]
    Start --> C["settotzero()\ncumprec = cumtra = cumeva\n= cumintc = masbal = 0"]
    Start --> D["pond = 0\nspnr = 0\n(surface ponding & soil-type counter)"]
    Start --> E["reset_canopy()\nClear interception store\nlastwet / wetsteps"]
    Start --> F["reset_timestep()\ndt = dtm1 = 0.01\nt = tm1 = 0"]

    A & B & C & D & E & F --> G["prelim()\nRead config, load forcing\nInit soil arrays and lookup tables\nInit canopy module"]

    G --> H["Ready for\nvamps_do_step(0 … N-1)"]
```

---

## 9. Forcing data flow

How forcing moves from a file or array into the C solver:

```mermaid
flowchart TD
    subgraph Sources
        S1["Disk: .prn / .ts\n2-column or multi-column"]
        S2["NumPy array\nprovided by caller"]
        S3[".inp file [ts] section\n'pre = all.inp,0,1'"]
    end

    S3 --> P1["forcing.py\nload_ts_spec(spec, base_dir)"]
    S1 --> P1
    S2 --> P2["_vampscore.so\nts_register_array(name, ptr, n, firststep)"]
    P1 --> P2

    P2 --> P3["ts_mem.c\nIn-memory registry\nts_arr_registry[]"]

    P3 --> P4["dataset.c : get_data(fname, name)\nchecks registry first\nfalls back to file read if absent"]

    P4 --> P5["data[id.pre].xy[i].{x,y}\ndata[id.ptr].xy[i].{x,y}\ndata[id.spe].xy[i].{x,y}\n…"]

    P5 --> P6["vamps_do_step(i)\ntstep_top reads & modifies .xy[i].y\ntstep_soil reads .xy[i].x and .y"]
```

---

## 10. Soil hydraulic model variants

The C soil module supports three water-retention / conductivity relationships,
selected per soil type via the `method` key in each `[st_N]` config section.

```mermaid
flowchart LR
    M["method in [st_N]"]
    M --> M0["0 · Clapp–Hornberger\nb, ψ_sat, θ_sat, K_sat"]
    M --> M1["1 · Van Genuchten  ★ most common\nα, n, l, θ_sat, θ_res, K_sat"]
    M --> M5["5 · User Python callbacks\n(HAVE_LIBPYTHON)\nh2t, t2k, h2dmc …"]

    M1 --> LUT["Look-up tables\n(mktable=1 in [soil])\nfilltab.c pre-computes\nθ(h), K(h), dθ/dh\nfor speed"]
    M0 --> LUT
```
