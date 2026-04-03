/*
 * vamps_ext.h — API for running VAMPS from a Python C extension.
 *
 * Provides:
 *   ts_register_array()   — register a numpy array as a forcing time series
 *   ts_clear_registry()   — reset the registry before each run
 *   vamps_run_ext()       — run the solver, collecting output in memory
 */

#ifndef VAMPS_EXT_H
#define VAMPS_EXT_H

/* Maximum number of soil layers (generous upper bound for pre-allocation). */
#define VAMPS_MAX_LAYERS 500

/* Register an in-memory forcing array to override file-based ts reading.
 * name      : ts variable name as it appears in the [ts] config section
 *             (e.g. "pre", "nra", "tem", ...)
 * vals      : array of double values, length n
 * n         : number of timesteps
 * firststep : x-coordinate of the first element (typically 1.0)
 */
void ts_register_array(const char *name, double *vals, int n, double firststep);

/* Clear all registered forcing arrays (call before each run). */
void ts_clear_registry(void);

/* Output struct — caller pre-allocates all arrays before calling vamps_run_ext. */
typedef struct {
    /* Input: sizes caller has allocated */
    int max_steps;
    int max_layers;   /* must be >= VAMPS_MAX_LAYERS or actual layer count */

    /* Output: filled in by vamps_run_ext */
    int actual_steps;
    int actual_layers;

    /* Per-step scalar outputs (length max_steps) */
    double *t;
    double *volact;
    double *SMD;
    double *qtop;
    double *qbot;
    double *avgtheta;
    double *cumprec;
    double *cumtra;
    double *cumeva;
    double *cumintc;
    double *masbal;
    double *precipitation;
    double *interception;
    double *transpiration;
    double *soilevaporation;

    /* Per-step profile outputs (row-major, may be NULL to skip collection)
     *
     *   theta, k, h, qrot, howsat : stride = max_layers      (length nlayers)
     *   q, inq                    : stride = max_layers + 1  (length nlayers+1)
     *   gwl                       : stride = 2               (always 2 values)
     */
    double *theta;   /* volumetric water content, length nlayers */
    double *k;       /* hydraulic conductivity, length nlayers */
    double *h;       /* pressure head, length nlayers */
    double *q;       /* inter-layer flux, length nlayers+1 */
    double *inq;     /* cumulative inter-layer flux, length nlayers+1 */
    double *qrot;    /* root water uptake, length nlayers */
    double *howsat;  /* degree of saturation, length nlayers */
    double *gwl;     /* groundwater table levels, always 2 */
} vamps_out_t;

/* Run the VAMPS solver.
 * config_file : path to a .inp file (written by the caller)
 * out         : pre-allocated output struct
 * Returns actual number of steps completed.
 */
int vamps_run_ext(const char *config_file, vamps_out_t *out);

/* Like vamps_run_ext() but accepts the config as an in-memory INI string.
 * No temp file is written or read.
 */
int vamps_run_ext_str(const char *ini_text, vamps_out_t *out);

#endif /* VAMPS_EXT_H */
