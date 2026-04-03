/*
 * vamps_ext.c — Direct solver entry point for the Python C extension.
 *
 * Provides vamps_run_ext() which replicates dorun() but writes output
 * into caller-supplied arrays instead of a .out file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vamps.h"
#include "swatsoil.h"
#include "deffile.h"            /* rinmem_buf */
#include "vamps_ext.h"
#include "../topsys/topsys.h"

/* Globals declared in vamps.c — not extern'd in any header */
extern char    infilename[];
extern char    outfilename[];
extern FILE   *genoutfile;
extern int     startpos;
extern double  starttime, endtime;
extern int     Dsoil;
extern int     doxtraout;
extern void    prelim(void);
extern void    del_all_sets(void);
extern void    reset_presoil(void);
extern void    settotzero(void);
extern void    reset_canopy(void);
extern void    reset_timestep(void);

/* Globals that need explicit reset between runs */
extern double pond;   /* surface ponding (soilboun.c) */
extern int    spnr;   /* soil-type counter (swatsoil.c) */

/* -------------------------------------------------------------------------
 * _reset_state — shared reset sequence for both entry points.
 * ---------------------------------------------------------------------- */
static void
_reset_state(void)
{
    del_all_sets();
    reset_presoil();
    settotzero();
    pond = 0.0;
    spnr = 0;
    reset_canopy();
    reset_timestep();

    if (genoutfile && genoutfile != stdout && genoutfile != stderr) {
        fclose(genoutfile);
        genoutfile = NULL;
    }
    strncpy(outfilename, "/dev/null", sizeof(outfilename) - 1);
    outfilename[sizeof(outfilename) - 1] = '\0';
}

/* -------------------------------------------------------------------------
 * _run_loop — timestep loop shared by both entry points.
 * Called after prelim() has already been run.
 * ---------------------------------------------------------------------- */
static int
_run_loop(vamps_out_t *out)
{
    int i, step;

    /* Record actual layer count (set by presoil() inside prelim()) */
    out->actual_layers = layers;

    /* Derive timestep length and loop bounds, same as dorun() */
    thiststep = getdefdoub("time", "firststep",
        data[id.pre].xy[1 + startpos].x - data[id.pre].xy[startpos].x,
        infilename, FALSE);
    t       = data[id.pre].xy[startpos].x - thiststep;
    starttime = t;
    endtime   = data[id.pre].xy[startpos + steps - 1].x;
    i         = startpos;
    step      = 0;

    while (t < endtime) {
        thisstep = i;
        if (i != startpos)
            thiststep = data[id.pre].xy[i].x - data[id.pre].xy[i - 1].x;

        /* Canopy / topsystem step */
        tstep_top(i,
                  &data[id.pre].xy[i].y,
                  &data[id.inr].xy[i].y,
                  &data[id.ptr].xy[i].y,
                  &data[id.spe].xy[i].y);

        /* Soil water flow step */
        if (Dsoil)
            tstep_soil(i,
                       data[id.pre].xy[i].x,
                       data[id.pre].xy[i].y,
                       data[id.inr].xy[i].y,
                       data[id.ptr].xy[i].y,
                       data[id.spe].xy[i].y);

        /* Collect per-step output into caller's arrays */
        if (step < out->max_steps) {
            out->t[step]              = data[id.pre].xy[i].x;
            out->volact[step]         = volact;
            out->SMD[step]            = SMD;
            out->qtop[step]           = qtop;
            out->qbot[step]           = qbot;
            out->avgtheta[step]       = avgtheta;
            out->cumprec[step]        = cumprec;
            out->cumtra[step]         = cumtra;
            out->cumeva[step]         = cumeva;
            out->cumintc[step]        = cumintc;
            out->masbal[step]         = masbal;
            out->precipitation[step]  = data[id.pre].xy[i].y;
            out->interception[step]   = data[id.inr].xy[i].y;
            out->transpiration[step]  = data[id.ptr].xy[i].y;
            out->soilevaporation[step]= data[id.spe].xy[i].y;

            {
                int ml  = out->max_layers;
                int ml1 = out->max_layers + 1;
                int nl  = (layers     < ml)  ? layers     : ml;
                int nl1 = (layers + 1 < ml1) ? layers + 1 : ml1;

                /* theta, k, h, qrot, howsat: length nlayers, stride ml */
                if (theta   && out->theta)
                    memcpy(out->theta  + (long)step * ml, theta,  nl * sizeof(double));
                if (k       && out->k)
                    memcpy(out->k      + (long)step * ml, k,      nl * sizeof(double));
                if (h       && out->h)
                    memcpy(out->h      + (long)step * ml, h,      nl * sizeof(double));
                if (qrot    && out->qrot)
                    memcpy(out->qrot   + (long)step * ml, qrot,   nl * sizeof(double));
                if (howsat  && out->howsat)
                    memcpy(out->howsat + (long)step * ml, howsat, nl * sizeof(double));
                /* q, inq: length nlayers+1, stride ml+1 */
                if (q       && out->q)
                    memcpy(out->q   + (long)step * (ml + 1), q,   nl1 * sizeof(double));
                if (inq     && out->inq)
                    memcpy(out->inq + (long)step * (ml + 1), inq, nl1 * sizeof(double));
                /* gwl: always 2 values */
                if (gwl     && out->gwl)
                    memcpy(out->gwl + (long)step * 2, gwl, 2 * sizeof(double));
            }
        }

        step++;
        i++;
        if (!Dsoil)
            t += thiststep;
    }

    out->actual_steps = step;
    return step;
}

/* -------------------------------------------------------------------------
 * vamps_run_ext — config read from a .inp file on disk.
 * ---------------------------------------------------------------------- */
int
vamps_run_ext(const char *config_file, vamps_out_t *out)
{
    _reset_state();
    strncpy(infilename, config_file, sizeof(infilename) - 1);
    infilename[sizeof(infilename) - 1] = '\0';
    prelim();
    return _run_loop(out);
}

/* -------------------------------------------------------------------------
 * vamps_run_ext_str — config supplied as an in-memory INI string.
 * No temp file is written or read.
 * ---------------------------------------------------------------------- */
int
vamps_run_ext_str(const char *ini_text, vamps_out_t *out)
{
    _reset_state();

    /* Pre-load config from string into the deffile in-memory store */
    if (!rinmem_buf("_vampscore_cfg_", ini_text))
        return -1;

    strncpy(infilename, "_vampscore_cfg_", sizeof(infilename) - 1);
    infilename[sizeof(infilename) - 1] = '\0';

    prelim();
    return _run_loop(out);
}
