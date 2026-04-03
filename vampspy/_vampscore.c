/*
 * _vampscore.c — Python C extension wrapping the VAMPS solver.
 *
 * Exposes: vampspy._vampscore.run(ini_text, forcing_dict, firststep=1)
 *   ini_text     : str   — VAMPS config as an INI-format string (no file needed)
 *   forcing_dict : dict  — {name: np.ndarray} for each [ts] variable
 *   firststep    : float — x-value of the first forcing element (default 1.0)
 *
 * Returns a dict of numpy arrays:
 *   scalar per-step : 't', 'volact', 'SMD', 'qtop', 'qbot', 'avgtheta',
 *                     'cumprec', 'cumtra', 'cumeva', 'cumintc', 'masbal',
 *                     'precipitation', 'interception', 'transpiration',
 *                     'soilevaporation'
 *   profiles (steps, nlayers)   : 'theta', 'k', 'h', 'qrot', 'howsat'
 *   profiles (steps, nlayers+1) : 'q', 'inq'
 *   profile  (steps, 2)         : 'gwl'
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* NumPy C API */
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <stdlib.h>
#include <string.h>

/* VAMPS extension API */
#include "vamps_ext.h"
#include "soil_api.h"

/* -------------------------------------------------------------------------
 * Helper: wrap a plain C double array as a numpy 1-D array (takes ownership
 * of the buffer via PyArray_SimpleNewFromData + capsule trick).
 * We copy into a fresh numpy array to keep things simple and safe.
 * ---------------------------------------------------------------------- */
static PyObject *
make_1d(double *buf, int n)
{
    npy_intp dims[1] = {n};
    PyObject *arr = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!arr) return NULL;
    memcpy(PyArray_DATA((PyArrayObject *)arr), buf, n * sizeof(double));
    return arr;
}

/* stride: number of doubles between successive rows in buf */
static PyObject *
make_2d(double *buf, int rows, int cols, int stride)
{
    npy_intp dims[2] = {rows, cols};
    PyObject *arr = PyArray_SimpleNew(2, dims, NPY_DOUBLE);
    if (!arr) return NULL;
    double *dst = (double *)PyArray_DATA((PyArrayObject *)arr);
    int r;
    for (r = 0; r < rows; r++)
        memcpy(dst + r * cols, buf + r * stride, cols * sizeof(double));
    return arr;
}

/* -------------------------------------------------------------------------
 * vampspy._vampscore.run(ini_text, forcing, firststep=1.0)
 * ---------------------------------------------------------------------- */
static PyObject *
py_run(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *ini_text;
    PyObject   *forcing_dict;
    double      firststep = 1.0;

    static char *kwlist[] = {"ini_text", "forcing", "firststep", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sO!|d", kwlist,
                                     &ini_text,
                                     &PyDict_Type, &forcing_dict,
                                     &firststep))
        return NULL;

    /* --- Register forcing arrays ---------------------------------------- */
    ts_clear_registry();

    /* Keep converted arrays alive until after prelim() runs */
    PyObject *live_refs = PyList_New(0);
    if (!live_refs) return NULL;

    int max_steps = 0;
    PyObject *key, *val;
    Py_ssize_t pos = 0;

    while (PyDict_Next(forcing_dict, &pos, &key, &val)) {
        const char *name = PyUnicode_AsUTF8(key);
        if (!name) { Py_DECREF(live_refs); return NULL; }

        /* Ensure contiguous C-order double array */
        PyObject *arr = PyArray_FROM_OTF(val, NPY_DOUBLE,
                                          NPY_ARRAY_IN_ARRAY |
                                          NPY_ARRAY_C_CONTIGUOUS);
        if (!arr) { Py_DECREF(live_refs); return NULL; }
        PyList_Append(live_refs, arr);
        Py_DECREF(arr);  /* list holds reference */

        int n = (int)PyArray_SIZE((PyArrayObject *)arr);
        if (n > max_steps) max_steps = n;

        ts_register_array(name,
                          (double *)PyArray_DATA((PyArrayObject *)arr),
                          n, firststep);
    }

    if (max_steps == 0) {
        Py_DECREF(live_refs);
        PyErr_SetString(PyExc_ValueError, "forcing dict is empty");
        return NULL;
    }

    /* --- Allocate output buffers ---------------------------------------- */
    vamps_out_t out;
    memset(&out, 0, sizeof(out));
    out.max_steps  = max_steps;
    out.max_layers = VAMPS_MAX_LAYERS;

#define ALLOC1D(field) \
    out.field = (double *)calloc(max_steps, sizeof(double)); \
    if (!out.field) { goto oom; }

    ALLOC1D(t);
    ALLOC1D(volact);
    ALLOC1D(SMD);
    ALLOC1D(qtop);
    ALLOC1D(qbot);
    ALLOC1D(avgtheta);
    ALLOC1D(cumprec);
    ALLOC1D(cumtra);
    ALLOC1D(cumeva);
    ALLOC1D(cumintc);
    ALLOC1D(masbal);
    ALLOC1D(precipitation);
    ALLOC1D(interception);
    ALLOC1D(transpiration);
    ALLOC1D(soilevaporation);
#undef ALLOC1D

    /* theta, k, h, qrot, howsat: length nlayers per step → stride VAMPS_MAX_LAYERS */
    out.theta  = (double *)calloc((size_t)max_steps * VAMPS_MAX_LAYERS, sizeof(double));
    if (!out.theta)  goto oom;
    out.k      = (double *)calloc((size_t)max_steps * VAMPS_MAX_LAYERS, sizeof(double));
    if (!out.k)      goto oom;
    out.h      = (double *)calloc((size_t)max_steps * VAMPS_MAX_LAYERS, sizeof(double));
    if (!out.h)      goto oom;
    out.qrot   = (double *)calloc((size_t)max_steps * VAMPS_MAX_LAYERS, sizeof(double));
    if (!out.qrot)   goto oom;
    out.howsat = (double *)calloc((size_t)max_steps * VAMPS_MAX_LAYERS, sizeof(double));
    if (!out.howsat) goto oom;
    /* q, inq: length nlayers+1 per step → stride VAMPS_MAX_LAYERS+1 */
    out.q      = (double *)calloc((size_t)max_steps * (VAMPS_MAX_LAYERS + 1), sizeof(double));
    if (!out.q)      goto oom;
    out.inq    = (double *)calloc((size_t)max_steps * (VAMPS_MAX_LAYERS + 1), sizeof(double));
    if (!out.inq)    goto oom;
    /* gwl: always 2 per step */
    out.gwl    = (double *)calloc((size_t)max_steps * 2, sizeof(double));
    if (!out.gwl)    goto oom;

    /* --- Run the solver -------------------------------------------------- */
    int nsteps = vamps_run_ext_str(ini_text, &out);

    /* live_refs kept alive through prelim(); can release now */
    Py_DECREF(live_refs);
    live_refs = NULL;

    /* --- Build result dict ----------------------------------------------- */
    int nl = out.actual_layers;
    PyObject *result = PyDict_New();
    if (!result) goto oom_post;

#define ADD1D(name, field) \
    { PyObject *a = make_1d(out.field, nsteps); \
      if (!a) { Py_DECREF(result); goto oom_post; } \
      PyDict_SetItemString(result, name, a); Py_DECREF(a); }

    ADD1D("t",               t);
    ADD1D("volact",          volact);
    ADD1D("SMD",             SMD);
    ADD1D("qtop",            qtop);
    ADD1D("qbot",            qbot);
    ADD1D("avgtheta",        avgtheta);
    ADD1D("cumprec",         cumprec);
    ADD1D("cumtra",          cumtra);
    ADD1D("cumeva",          cumeva);
    ADD1D("cumintc",         cumintc);
    ADD1D("masbal",          masbal);
    ADD1D("precipitation",   precipitation);
    ADD1D("interception",    interception);
    ADD1D("transpiration",   transpiration);
    ADD1D("soilevaporation", soilevaporation);
#undef ADD1D

    /* Profile arrays */
#define ADD2D(name, field, cols, stride) \
    { PyObject *a = make_2d(out.field, nsteps, cols, stride); \
      if (!a) { Py_DECREF(result); goto oom_post; } \
      PyDict_SetItemString(result, name, a); Py_DECREF(a); }

    ADD2D("theta",  theta,  nl,      VAMPS_MAX_LAYERS)
    ADD2D("k",      k,      nl,      VAMPS_MAX_LAYERS)
    ADD2D("h",      h,      nl,      VAMPS_MAX_LAYERS)
    ADD2D("q",      q,      nl + 1,  VAMPS_MAX_LAYERS + 1)
    ADD2D("inq",    inq,    nl + 1,  VAMPS_MAX_LAYERS + 1)
    ADD2D("qrot",   qrot,   nl,      VAMPS_MAX_LAYERS)
    ADD2D("howsat", howsat, nl,      VAMPS_MAX_LAYERS)
    ADD2D("gwl",    gwl,    2,       2)
#undef ADD2D

    /* metadata */
    {
        PyObject *v = PyLong_FromLong(nsteps);
        PyDict_SetItemString(result, "_steps", v); Py_DECREF(v);
        v = PyLong_FromLong(nl);
        PyDict_SetItemString(result, "_nlayers", v); Py_DECREF(v);
    }

#define FREE_OUT \
    free(out.t); free(out.volact); free(out.SMD); \
    free(out.qtop); free(out.qbot); free(out.avgtheta); \
    free(out.cumprec); free(out.cumtra); free(out.cumeva); \
    free(out.cumintc); free(out.masbal); \
    free(out.precipitation); free(out.interception); \
    free(out.transpiration); free(out.soilevaporation); \
    free(out.theta); free(out.k); free(out.h); \
    free(out.q); free(out.inq); free(out.qrot); \
    free(out.howsat); free(out.gwl)

    FREE_OUT;
    return result;

oom:
    Py_XDECREF(live_refs);
oom_post:
    FREE_OUT;
#undef FREE_OUT
    return PyErr_NoMemory();
}

/* -------------------------------------------------------------------------
 * Stepwise API — module-level state
 *
 * _sw_live_refs holds references to the numpy forcing arrays registered with
 * ts_register_array so they stay alive for the lifetime of a stepwise run.
 * ---------------------------------------------------------------------- */
static PyObject *_sw_live_refs = NULL;

/* -------------------------------------------------------------------------
 * vampspy._vampscore.soil_init(ini_text, forcing, firststep=1.0)
 *
 * Initialise a stepwise run: register forcing arrays, call prelim().
 * Must be called once before any soil_step() calls.
 * ---------------------------------------------------------------------- */
static PyObject *
py_soil_init(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *ini_text;
    PyObject   *forcing_dict;
    double      firststep = 1.0;

    static char *kwlist[] = {"ini_text", "forcing", "firststep", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sO!|d", kwlist,
                                     &ini_text,
                                     &PyDict_Type, &forcing_dict,
                                     &firststep))
        return NULL;

    /* Release refs from any previous stepwise run */
    Py_XDECREF(_sw_live_refs);
    _sw_live_refs = PyList_New(0);
    if (!_sw_live_refs) return NULL;

    ts_clear_registry();

    PyObject *key, *val;
    Py_ssize_t pos = 0;
    while (PyDict_Next(forcing_dict, &pos, &key, &val)) {
        const char *name = PyUnicode_AsUTF8(key);
        if (!name) return NULL;

        PyObject *arr = PyArray_FROM_OTF(val, NPY_DOUBLE,
                                          NPY_ARRAY_IN_ARRAY |
                                          NPY_ARRAY_C_CONTIGUOUS);
        if (!arr) return NULL;
        PyList_Append(_sw_live_refs, arr);
        Py_DECREF(arr);  /* list holds the reference */

        int n = (int)PyArray_SIZE((PyArrayObject *)arr);
        ts_register_array(name,
                          (double *)PyArray_DATA((PyArrayObject *)arr),
                          n, firststep);
    }

    if (vamps_init_stepwise_str(ini_text) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "vamps_init_stepwise_str failed");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* -------------------------------------------------------------------------
 * vampspy._vampscore.soil_step(step)
 *
 * Execute one external timestep (0-based index).
 * Calls tstep_top (C canopy) then tstep_soil (Richards solver).
 * ---------------------------------------------------------------------- */
static PyObject *
py_soil_step(PyObject *self, PyObject *args)
{
    int step;
    if (!PyArg_ParseTuple(args, "i", &step)) return NULL;

    if (vamps_do_step(step) != 0) {
        PyErr_Format(PyExc_RuntimeError,
                     "vamps_do_step failed at step %d (timestep <= 0?)", step);
        return NULL;
    }
    Py_RETURN_NONE;
}

/* -------------------------------------------------------------------------
 * vampspy._vampscore.soil_state(step) -> dict
 *
 * Return a dict of scalar outputs plus a 1-D numpy 'theta' array for the
 * most recently completed step.
 * ---------------------------------------------------------------------- */
static PyObject *
py_soil_state(PyObject *self, PyObject *args)
{
    int step;
    if (!PyArg_ParseTuple(args, "i", &step)) return NULL;

    vamps_state_t    st;
    vamps_profiles_t prof;
    vamps_get_state(&st, step);
    vamps_get_profiles(&prof);

    int nl = st.nlayers;
    double *theta_buf = (double *)malloc(nl * sizeof(double));
    if (!theta_buf) return PyErr_NoMemory();
    vamps_get_theta(theta_buf, nl);

    PyObject *result = PyDict_New();
    if (!result) { free(theta_buf); return NULL; }

#define SW_ADD_DOUBLE(key, val) \
    do { PyObject *_v = PyFloat_FromDouble(val); \
         if (!_v) { Py_DECREF(result); free(theta_buf); return NULL; } \
         PyDict_SetItemString(result, key, _v); Py_DECREF(_v); } while (0)

    SW_ADD_DOUBLE("t",               st.t);
    SW_ADD_DOUBLE("volact",          st.volact);
    SW_ADD_DOUBLE("SMD",             st.SMD);
    SW_ADD_DOUBLE("qtop",            st.qtop);
    SW_ADD_DOUBLE("qbot",            st.qbot);
    SW_ADD_DOUBLE("avgtheta",        st.avgtheta);
    SW_ADD_DOUBLE("cumprec",         st.cumprec);
    SW_ADD_DOUBLE("cumtra",          st.cumtra);
    SW_ADD_DOUBLE("cumeva",          st.cumeva);
    SW_ADD_DOUBLE("cumintc",         st.cumintc);
    SW_ADD_DOUBLE("masbal",          st.masbal);
    SW_ADD_DOUBLE("precipitation",   st.precipitation);
    SW_ADD_DOUBLE("interception",    st.interception);
    SW_ADD_DOUBLE("transpiration",   st.transpiration);
    SW_ADD_DOUBLE("soilevaporation", st.soilevaporation);
#undef SW_ADD_DOUBLE

    { PyObject *v = PyLong_FromLong(nl);
      PyDict_SetItemString(result, "_nlayers", v); Py_DECREF(v); }

#define SW_ADD_PROFILE(key, arr, n) \
    do { npy_intp _d[1] = {(n)}; \
         PyObject *_a = PyArray_SimpleNew(1, _d, NPY_DOUBLE); \
         if (!_a) { Py_DECREF(result); free(theta_buf); return NULL; } \
         memcpy(PyArray_DATA((PyArrayObject *)_a), (arr), (n) * sizeof(double)); \
         PyDict_SetItemString(result, (key), _a); Py_DECREF(_a); } while (0)

    SW_ADD_PROFILE("theta",  theta_buf,   nl);
    SW_ADD_PROFILE("k",      prof.k,      nl);
    SW_ADD_PROFILE("h",      prof.h,      nl);
    SW_ADD_PROFILE("q",      prof.q,      nl + 1);
    SW_ADD_PROFILE("inq",    prof.inq,    nl + 1);
    SW_ADD_PROFILE("qrot",   prof.qrot,   nl);
    SW_ADD_PROFILE("howsat", prof.howsat, nl);
    SW_ADD_PROFILE("gwl",    prof.gwl,    2);
#undef SW_ADD_PROFILE

    free(theta_buf);
    return result;
}

/* -------------------------------------------------------------------------
 * vampspy._vampscore.soil_nlayers() -> int
 * ---------------------------------------------------------------------- */
static PyObject *
py_soil_nlayers(PyObject *self, PyObject *args)
{
    return PyLong_FromLong(vamps_nlayers());
}

/* -------------------------------------------------------------------------
 * vampspy._vampscore.soil_step_direct(pre, intc=0, ptra=0, peva=0, rdp=0)
 *
 * Execute one soil timestep with explicit per-step forcing scalars.
 * Bypasses the C canopy (tstep_top) — the caller supplies post-canopy
 * values directly.  Uses the same soil_init() session as soil_step().
 * ---------------------------------------------------------------------- */
static PyObject *
py_soil_step_direct(PyObject *self, PyObject *args, PyObject *kwargs)
{
    double pre, intc = 0.0, ptra = 0.0, peva = 0.0, rdp = 0.0;
    static char *kwlist[] = {"pre", "intc", "ptra", "peva", "rdp", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d|dddd", kwlist,
                                     &pre, &intc, &ptra, &peva, &rdp))
        return NULL;

    if (vamps_do_step_direct(pre, intc, ptra, peva, rdp) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "soil_step_direct: timestep interval <= 0");
        return NULL;
    }
    Py_RETURN_NONE;
}

/* -------------------------------------------------------------------------
 * vampspy._vampscore.soil_state_current() -> dict
 *
 * Return the scalar state and per-layer profile arrays after the most
 * recently completed soil_step_direct() call.  No step index required.
 * ---------------------------------------------------------------------- */
static PyObject *
py_soil_state_current(PyObject *self, PyObject *args)
{
    vamps_state_t    st;
    vamps_profiles_t prof;
    vamps_get_state_current(&st);
    vamps_get_profiles(&prof);

    int nl = st.nlayers;
    double *theta_buf = (double *)malloc(nl * sizeof(double));
    if (!theta_buf) return PyErr_NoMemory();
    vamps_get_theta(theta_buf, nl);

    PyObject *result = PyDict_New();
    if (!result) { free(theta_buf); return NULL; }

#define CUR_ADD_DOUBLE(key, val) \
    do { PyObject *_v = PyFloat_FromDouble(val); \
         if (!_v) { Py_DECREF(result); free(theta_buf); return NULL; } \
         PyDict_SetItemString(result, key, _v); Py_DECREF(_v); } while (0)

    CUR_ADD_DOUBLE("t",               st.t);
    CUR_ADD_DOUBLE("volact",          st.volact);
    CUR_ADD_DOUBLE("SMD",             st.SMD);
    CUR_ADD_DOUBLE("qtop",            st.qtop);
    CUR_ADD_DOUBLE("qbot",            st.qbot);
    CUR_ADD_DOUBLE("avgtheta",        st.avgtheta);
    CUR_ADD_DOUBLE("cumprec",         st.cumprec);
    CUR_ADD_DOUBLE("cumtra",          st.cumtra);
    CUR_ADD_DOUBLE("cumeva",          st.cumeva);
    CUR_ADD_DOUBLE("cumintc",         st.cumintc);
    CUR_ADD_DOUBLE("masbal",          st.masbal);
    CUR_ADD_DOUBLE("precipitation",   st.precipitation);
    CUR_ADD_DOUBLE("interception",    st.interception);
    CUR_ADD_DOUBLE("transpiration",   st.transpiration);
    CUR_ADD_DOUBLE("soilevaporation", st.soilevaporation);
#undef CUR_ADD_DOUBLE

    { PyObject *v = PyLong_FromLong(nl);
      PyDict_SetItemString(result, "_nlayers", v); Py_DECREF(v); }

#define CUR_ADD_PROFILE(key, arr, n) \
    do { npy_intp _d[1] = {(n)}; \
         PyObject *_a = PyArray_SimpleNew(1, _d, NPY_DOUBLE); \
         if (!_a) { Py_DECREF(result); free(theta_buf); return NULL; } \
         memcpy(PyArray_DATA((PyArrayObject *)_a), (arr), (n) * sizeof(double)); \
         PyDict_SetItemString(result, (key), _a); Py_DECREF(_a); } while (0)

    CUR_ADD_PROFILE("theta",  theta_buf,   nl);
    CUR_ADD_PROFILE("k",      prof.k,      nl);
    CUR_ADD_PROFILE("h",      prof.h,      nl);
    CUR_ADD_PROFILE("q",      prof.q,      nl + 1);
    CUR_ADD_PROFILE("inq",    prof.inq,    nl + 1);
    CUR_ADD_PROFILE("qrot",   prof.qrot,   nl);
    CUR_ADD_PROFILE("howsat", prof.howsat, nl);
    CUR_ADD_PROFILE("gwl",    prof.gwl,    2);
#undef CUR_ADD_PROFILE

    free(theta_buf);
    return result;
}

/* -------------------------------------------------------------------------
 * Module definition
 * ---------------------------------------------------------------------- */
static PyMethodDef vampscore_methods[] = {
    {"run", (PyCFunction)py_run, METH_VARARGS | METH_KEYWORDS,
     "run(ini_text, forcing, firststep=1.0) -> dict\n\n"
     "Run VAMPS directly without temp files.\n"
     "Returns a dict of numpy arrays.\n"},
    /* --- stepwise API --- */
    {"soil_init", (PyCFunction)py_soil_init, METH_VARARGS | METH_KEYWORDS,
     "soil_init(ini_text, forcing, firststep=1.0)\n\n"
     "Initialise a stepwise run.  Call once; then loop soil_step/soil_state.\n"},
    {"soil_step", py_soil_step, METH_VARARGS,
     "soil_step(step)\n\n"
     "Execute one external timestep (0-based index).\n"
     "Runs C canopy (tstep_top) then Richards solver (tstep_soil).\n"},
    {"soil_state", py_soil_state, METH_VARARGS,
     "soil_state(step) -> dict\n\n"
     "Return scalar state + 1-D theta array after the most recent step.\n"},
    {"soil_nlayers", py_soil_nlayers, METH_NOARGS,
     "soil_nlayers() -> int\n\nNumber of soil layers (available after soil_init).\n"},
    /* --- direct (thin) API --- */
    {"soil_step_direct", (PyCFunction)py_soil_step_direct, METH_VARARGS | METH_KEYWORDS,
     "soil_step_direct(pre, intc=0.0, ptra=0.0, peva=0.0, rdp=0.0)\n\n"
     "Execute one soil timestep with explicit per-step forcing scalars.\n"
     "Bypasses the C canopy (tstep_top) — caller provides post-canopy values.\n"
     "Must be preceded by soil_init().\n"},
    {"soil_state_current", py_soil_state_current, METH_NOARGS,
     "soil_state_current() -> dict\n\n"
     "Return scalar state + per-layer profile arrays after the most recently\n"
     "completed soil_step_direct() call.  No step index needed.\n"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef vampscore_module = {
    PyModuleDef_HEAD_INIT, "_vampscore", NULL, -1, vampscore_methods
};

PyMODINIT_FUNC
PyInit__vampscore(void)
{
    import_array();  /* initialise NumPy C API */
    return PyModule_Create(&vampscore_module);
}
