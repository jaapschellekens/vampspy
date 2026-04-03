/*
 * soil_api.c — Per-step C API for Python-driven VAMPS runs.
 *
 * Provides vamps_init_stepwise / vamps_do_step / vamps_get_state /
 * vamps_get_theta / vamps_nlayers so that the outer timestep loop can
 * live in Python while the soil+canopy physics stays in C.
 */

#include <stdio.h>
#include <string.h>

#include "soil_api.h"
#include "vamps.h"
#include "swatsoil.h"
#include "vamps_ext.h"          /* ts_clear_registry, ts_register_array */
#include "deffile.h"            /* getdefdoub, rinmem_buf */
#include "../topsys/topsys.h"  /* tstep_top */

/* -------------------------------------------------------------------------
 * Globals from vamps.c that we need but are not in any header.
 * ---------------------------------------------------------------------- */
extern char    infilename[];
extern char    outfilename[];
extern FILE   *genoutfile;
extern int     startpos;
extern double  starttime, endtime;
extern int     Dsoil;
extern int     steps;
extern double  thiststep;
extern int     thisstep;
extern dataset *data;
extern ID      id;

extern void    prelim(void);
extern void    del_all_sets(void);
extern void    reset_presoil(void);
extern void    settotzero(void);
extern void    reset_canopy(void);
extern void    reset_timestep(void);

/* From swatsoil.c / soilboun.c */
extern double pond;
extern int    spnr;

/* -------------------------------------------------------------------------
 * Module-level flags.
 * sw_first_call : tracks whether step-0 time initialisation has been done.
 * sw_step_np    : step counter for vamps_do_step_direct().
 * ---------------------------------------------------------------------- */
static int sw_first_call  = 1;
static int sw_step_np     = 0;   /* counter for vamps_do_step_direct */
static int sw_last_step_i = -1;  /* absolute index of last completed step */

/* -------------------------------------------------------------------------
 * vamps_init_stepwise
 * ---------------------------------------------------------------------- */
int
vamps_init_stepwise(const char *config_file)
{
    /* Reset all inter-run state (mirrors vamps_run_ext) */
    del_all_sets();
    reset_presoil();
    settotzero();
    pond  = 0.0;
    spnr  = 0;
    reset_canopy();
    reset_timestep();
    sw_first_call  = 1;
    sw_step_np     = 0;
    sw_last_step_i = -1;

    /* Redirect output to /dev/null so prelim() doesn't open a real .out */
    if (genoutfile && genoutfile != stdout && genoutfile != stderr) {
        fclose(genoutfile);
        genoutfile = NULL;
    }
    strncpy(infilename,  config_file, sizeof(infilename)  - 1);
    infilename[sizeof(infilename)  - 1] = '\0';
    strncpy(outfilename, "/dev/null", sizeof(outfilename) - 1);
    outfilename[sizeof(outfilename) - 1] = '\0';

    /* Read config, initialise canopy and soil modules, load ts datasets */
    prelim();

    return 0;
}

/* -------------------------------------------------------------------------
 * vamps_init_stepwise_str
 * ---------------------------------------------------------------------- */
int
vamps_init_stepwise_str(const char *ini_text)
{
#define _VAMPSCORE_CFG_KEY "_vampscore_cfg_"

    del_all_sets();
    reset_presoil();
    settotzero();
    pond  = 0.0;
    spnr  = 0;
    reset_canopy();
    reset_timestep();
    sw_first_call  = 1;
    sw_step_np     = 0;
    sw_last_step_i = -1;

    /* Pre-load config from string into the deffile in-memory store.
     * Every subsequent getdefint/getdefstr call uses this cache;
     * no file is opened or written. */
    if (!rinmem_buf(_VAMPSCORE_CFG_KEY, ini_text))
        return -1;

    strncpy(infilename,  _VAMPSCORE_CFG_KEY, sizeof(infilename)  - 1);
    infilename[sizeof(infilename)  - 1] = '\0';

    if (genoutfile && genoutfile != stdout && genoutfile != stderr) {
        fclose(genoutfile);
        genoutfile = NULL;
    }
    strncpy(outfilename, "/dev/null", sizeof(outfilename) - 1);
    outfilename[sizeof(outfilename) - 1] = '\0';

    prelim();
    return 0;

#undef _VAMPSCORE_CFG_KEY
}

/* -------------------------------------------------------------------------
 * vamps_do_step
 * ---------------------------------------------------------------------- */
int
vamps_do_step(int step)
{
    int i = startpos + step;

    /* First call: initialise time variables (mirrors dorun()) */
    if (sw_first_call) {
        thiststep = getdefdoub("time", "firststep",
                               data[id.pre].xy[1 + startpos].x
                               - data[id.pre].xy[startpos].x,
                               infilename, FALSE);
        t         = data[id.pre].xy[startpos].x - thiststep;
        starttime = t;
        endtime   = data[id.pre].xy[startpos + steps - 1].x;
        sw_first_call = 0;
    } else {
        thiststep = data[id.pre].xy[i].x - data[id.pre].xy[i - 1].x;
        if (thiststep <= 0.0)
            return -1;
    }

    thisstep = i;

    /* Canopy step (modifies pre/inr/ptr/spe in the dataset arrays in-place) */
    tstep_top(i,
              &data[id.pre].xy[i].y,
              &data[id.inr].xy[i].y,
              &data[id.ptr].xy[i].y,
              &data[id.spe].xy[i].y);

    /* Soil step: Richards solver with adaptive internal sub-stepping */
    if (Dsoil)
        tstep_soil(i,
                   data[id.pre].xy[i].x,
                   data[id.pre].xy[i].y,
                   data[id.inr].xy[i].y,
                   data[id.ptr].xy[i].y,
                   data[id.spe].xy[i].y);

    sw_last_step_i = i;
    return 0;
}

/* -------------------------------------------------------------------------
 * vamps_get_state
 * ---------------------------------------------------------------------- */
void
vamps_get_state(vamps_state_t *out, int step)
{
    int i = startpos + step;

    out->t               = data[id.pre].xy[i].x;
    out->volact          = volact;
    out->SMD             = SMD;
    out->qtop            = qtop;
    out->qbot            = qbot;
    out->avgtheta        = avgtheta;
    out->cumprec         = cumprec;
    out->cumtra          = cumtra;
    out->cumeva          = cumeva;
    out->cumintc         = cumintc;
    out->masbal          = masbal;
    out->precipitation   = data[id.pre].xy[i].y;
    out->interception    = data[id.inr].xy[i].y;
    out->transpiration   = data[id.ptr].xy[i].y;
    out->soilevaporation = data[id.spe].xy[i].y;
    out->nlayers         = layers;
}

/* -------------------------------------------------------------------------
 * vamps_get_theta
 * ---------------------------------------------------------------------- */
void
vamps_get_theta(double *buf, int n)
{
    int j, nl = (layers < n) ? layers : n;
    for (j = 0; j < nl; j++)
        buf[j] = theta[j];
}

/* -------------------------------------------------------------------------
 * vamps_get_profiles
 * ---------------------------------------------------------------------- */
void
vamps_get_profiles(vamps_profiles_t *out)
{
    int j;
    int nl  = layers;
    int nl1 = layers + 1;

    /* k, h, qrot, howsat: length layers (same as theta) */
    for (j = 0; j < nl; j++) {
        out->k[j]      = k[j];
        out->h[j]      = h[j];
        out->qrot[j]   = qrot[j];
        out->howsat[j] = howsat[j];
    }
    /* q, inq: length layers+1 */
    for (j = 0; j < nl1; j++) {
        out->q[j]   = q[j];
        out->inq[j] = inq[j];
    }
    out->gwl[0] = gwl[0];
    out->gwl[1] = gwl[1];
}

/* -------------------------------------------------------------------------
 * vamps_nlayers
 * ---------------------------------------------------------------------- */
int
vamps_nlayers(void)
{
    return layers;
}

/* -------------------------------------------------------------------------
 * vamps_do_step_direct
 *
 * Like vamps_do_step() but accepts per-step forcing scalars directly.
 * Patches the dataset arrays at the current step index, then calls
 * tstep_soil() without going through tstep_top() — the caller is
 * responsible for any canopy calculation.
 *
 * Arguments:
 *   pre   — net precipitation reaching the soil surface [cm/day]
 *   intc  — interception by canopy [cm/day]
 *   ptra  — potential transpiration [cm/day]
 *   peva  — potential soil evaporation [cm/day]
 *   rdp   — root depth [cm] (ignored when id.rdp < 0)
 *
 * Returns 0 on success, -1 if the timestep interval is <= 0.
 * ---------------------------------------------------------------------- */
int
vamps_do_step_direct(double pre, double intc, double ptra, double peva, double rdp)
{
    int i = startpos + sw_step_np;

    /* Initialise time variables on the first call — mirrors vamps_do_step,
     * uses the shared sw_first_call flag so both step functions stay in sync. */
    if (sw_first_call) {
        thiststep = getdefdoub("time", "firststep",
                               data[id.pre].xy[1 + startpos].x
                               - data[id.pre].xy[startpos].x,
                               infilename, FALSE);
        t         = data[id.pre].xy[startpos].x - thiststep;
        starttime = t;
        endtime   = data[id.pre].xy[startpos + steps - 1].x;
        sw_first_call = 0;
    } else {
        thiststep = data[id.pre].xy[i].x - data[id.pre].xy[i - 1].x;
        if (thiststep <= 0.0)
            return -1;
    }
    thisstep = i;

    /* Patch dataset arrays so tstep_soil reads the caller-supplied values */
    data[id.pre].xy[i].y = pre;
    data[id.inr].xy[i].y = intc;
    data[id.ptr].xy[i].y = ptra;
    data[id.spe].xy[i].y = peva;
    if (id.rdp >= 0)
        data[id.rdp].xy[i].y = rdp;

    /* Soil physics — canopy step is deliberately skipped here */
    if (Dsoil)
        tstep_soil(i,
                   data[id.pre].xy[i].x,  /* time coordinate */
                   pre, intc, ptra, peva);

    sw_last_step_i = i;
    sw_step_np++;
    return 0;
}

/* -------------------------------------------------------------------------
 * vamps_get_state_current
 *
 * Fill *out with the scalar state after the most recently completed
 * vamps_do_step_direct() call (no step index required).
 * ---------------------------------------------------------------------- */
void
vamps_get_state_current(vamps_state_t *out)
{
    vamps_get_state(out, sw_last_step_i);
}
