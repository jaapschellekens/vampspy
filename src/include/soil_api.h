/*
 * soil_api.h — Per-step C API for Python-driven VAMPS runs.
 *
 * Exposes the soil+canopy physics as init-once / step-by-step calls so that
 * the outer (external-timestep) loop can live in Python while the solver
 * itself — Richards equation, adaptive sub-stepping, canopy water balance —
 * remains in C.
 */

#ifndef SOIL_API_H
#define SOIL_API_H

#include "vamps_ext.h"  /* VAMPS_MAX_LAYERS */

/*
 * Snapshot of per-step scalar output.  Profile output (theta) is handled
 * separately via vamps_get_theta().
 */
typedef struct {
    double t;                /* absolute time of this step */
    double volact;           /* actual water volume in profile [cm] */
    double SMD;              /* soil moisture deficit [cm] */
    double qtop;             /* flux at top of profile [cm/day] */
    double qbot;             /* flux at bottom of profile [cm/day] */
    double avgtheta;         /* average water content [-] */
    double cumprec;          /* cumulative precipitation [cm] */
    double cumtra;           /* cumulative transpiration [cm] */
    double cumeva;           /* cumulative soil evaporation [cm] */
    double cumintc;          /* cumulative interception [cm] */
    double masbal;           /* mass balance error [cm] */
    double precipitation;    /* precipitation this step (after canopy) [cm] */
    double interception;     /* interception this step [cm] */
    double transpiration;    /* transpiration this step [cm] */
    double soilevaporation;  /* soil evaporation this step [cm] */
    int    nlayers;          /* number of soil layers */
} vamps_state_t;

/*
 * vamps_init_stepwise(config_file)
 *
 * Reset all state, read the config file, load forcing datasets (via the
 * ts_register_array registry if arrays have been registered), initialise
 * the canopy and soil modules.  Must be called once before any vamps_do_step.
 *
 * Returns 0 on success, -1 on error.
 */
int  vamps_init_stepwise(const char *config_file);

/*
 * vamps_init_stepwise_str(ini_text)
 *
 * Like vamps_init_stepwise() but accepts the config as an in-memory string
 * instead of a file path.  No temp file is written or read.
 *
 * Returns 0 on success, -1 on error.
 */
int  vamps_init_stepwise_str(const char *ini_text);

/*
 * vamps_do_step(step)
 *
 * Execute one external timestep (0-based index).  Internally calls
 * tstep_top() for the canopy then tstep_soil() for the Richards solver
 * (with its own adaptive internal sub-stepping).
 *
 * Returns 0 on success, -1 if the timestep length is <= 0.
 */
int  vamps_do_step(int step);

/*
 * vamps_get_state(out, step)
 *
 * Fill *out with the scalar state after the most recently completed step.
 * `step` must be the same index passed to the preceding vamps_do_step call.
 */
void vamps_get_state(vamps_state_t *out, int step);

/*
 * vamps_get_theta(buf, n)
 *
 * Copy the current theta (volumetric water content) profile into buf[0..n-1].
 * At most min(nlayers, n) values are written.
 */
void vamps_get_theta(double *buf, int n);

/*
 * Per-step per-layer profile output.
 *
 * Array sizes:
 *   k, q, inq, qrot  : nlayers + 1
 *   h, howsat         : nlayers
 *   gwl               : 2 (groundwater table levels, always)
 */
/*
 * Array lengths match soilout.c / alloc.c:
 *   length layers   : k, h, qrot, howsat
 *   length layers+1 : q, inq
 *   always 2        : gwl
 */
typedef struct {
    double k     [VAMPS_MAX_LAYERS];     /* hydraulic conductivity, length layers */
    double h     [VAMPS_MAX_LAYERS];     /* pressure head, length layers */
    double q     [VAMPS_MAX_LAYERS + 1]; /* inter-layer flux, length layers+1 */
    double inq   [VAMPS_MAX_LAYERS + 1]; /* cumulative inter-layer flux, length layers+1 */
    double qrot  [VAMPS_MAX_LAYERS];     /* root water uptake, length layers */
    double howsat[VAMPS_MAX_LAYERS];     /* degree of saturation, length layers */
    double gwl   [2];                    /* groundwater table levels */
} vamps_profiles_t;

/*
 * vamps_get_profiles(out)
 *
 * Fill *out with all per-layer profile arrays after the most recently
 * completed step.  Must be called after vamps_do_step().
 */
void vamps_get_profiles(vamps_profiles_t *out);

/*
 * vamps_nlayers()
 *
 * Return the number of soil layers (set after vamps_init_stepwise).
 */
int  vamps_nlayers(void);

/*
 * vamps_do_step_direct(pre, intc, ptra, peva, rdp)
 *
 * Like vamps_do_step() but accepts per-step forcing scalars directly.
 * Patches the internal dataset arrays then calls tstep_soil() without
 * going through tstep_top() — the caller handles any canopy calculation.
 *
 *   pre  — net precipitation at soil surface [cm/day]
 *   intc — interception [cm/day]
 *   ptra — potential transpiration [cm/day]
 *   peva — potential soil evaporation [cm/day]
 *   rdp  — root depth [cm]
 *
 * Returns 0 on success, -1 if the timestep interval is <= 0.
 */
int  vamps_do_step_direct(double pre, double intc, double ptra, double peva, double rdp);

/*
 * vamps_get_state_current(out)
 *
 * Fill *out with the scalar state after the most recently completed
 * vamps_do_step_direct() call.  No step index needed.
 */
void vamps_get_state_current(vamps_state_t *out);

/*
 * vamps_patch_ts(name, step, value)
 *
 * Overwrite data[id.X].xy[startpos+step].y for the named ts variable.
 * Call before vamps_do_step() to supply per-step forcing without
 * pre-loading the full series at init time.
 *
 * Returns 0 on success, -1 if name not found, -2 if step out of range.
 */
int vamps_patch_ts(const char *name, int step, double value);

#endif /* SOIL_API_H */
