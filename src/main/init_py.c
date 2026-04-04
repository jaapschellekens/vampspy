/*
 * init_py.c — Python embedding for VAMPS
 *
 * Provides a built-in "vamps" Python module that exposes simulation state,
 * and implements the at_start / each_step / at_end callback mechanism.
 *
 * Usage from user scripts:
 *
 *   import vamps
 *
 *   def at_start():
 *       print("Starting, steps =", vamps.steps)
 *       vamps.verbose = 2
 *
 *   def each_step():
 *       print(vamps.t, vamps.volact)
 *
 *   def at_end():
 *       print("Done, cumprec =", vamps.cumprec)
 */

#ifdef HAVE_LIBPYTHON

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include "vamps.h"
#include "swatsoil.h"
#include "canopy.h"   /* VPD, ea, es */
#include "deffile.h"  /* getdefstr */

/* -----------------------------------------------------------------------
 * External C globals that we expose to Python
 * Most are declared in vamps.h / swatsoil.h / canopy.h already.
 * ----------------------------------------------------------------------- */

/* vamps.h */
extern int    verbose;
extern char   graphcommand[];
extern char   vampslib[];
extern ID     id;

extern void   printsum(FILE *thef);
extern void   printstr(const char *des, const char *str);
extern double cpusec(void);

/* data[] is the global time-series store declared in vamps.h */
/* We access it directly to avoid the HAVE_LIBSLANG dependency */
extern int  sets;
extern int  getsetbyname(char *s);

/* ---------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
 * Built-in "vamps" module
 *
 * We update the module's __dict__ with current simulation state before
 * every callback, and read back writable variables afterwards.
 * ----------------------------------------------------------------------- */

/* Push current simulation state into the vamps module dictionary. */
static void update_state(PyObject *dict)
{
#define SETD(name, val)  PyDict_SetItemString(dict, name, PyFloat_FromDouble(val))
#define SETI(name, val)  PyDict_SetItemString(dict, name, PyLong_FromLong((long)(val)))
#define SETS(name, val)  PyDict_SetItemString(dict, name, PyUnicode_FromString(val))

    /* Time */
    SETD("t",              t);
    SETD("thiststep",      thiststep);
    SETI("thisstep",       thisstep);
    SETI("steps",          steps);

    /* Soil water balance */
    SETD("masbal",         masbal);
    SETD("cumtop",         cumtop);
    SETD("cumbot",         cumbot);
    SETD("rootextract",    rootextract);
    SETD("cumeva",         cumeva);
    SETD("cumdra",         cumdra);
    SETD("cumtra",         cumtra);
    SETD("cumprec",        cumprec);
    SETD("cumintc",        cumintc);
    SETD("volact",         volact);
    SETD("volsat",         volsat);
    SETD("surface_runoff", surface_runoff);
    SETD("ptra",           ptra);
    SETD("qdrtot",         qdrtot);
    SETD("avgtheta",       avgtheta);
    SETD("SMD",            SMD);
    SETD("qtop",           qtop);
    SETD("qbot",           qbot);
    SETI("layers",         layers);

    /* Atmosphere */
    SETD("VPD",            VPD);

    /* Configuration (writable) */
    SETI("verbose",        verbose);
    SETI("soilverb",       soilverb);
    SETD("thetol",         thetol);
    SETI("maxitr",         maxitr);

    /* Run info */
    SETS("infilename",     infilename);
    SETS("outfilename",    outfilename);

    /* Number of loaded input datasets */
    SETI("sets", sets);

    /* Input time-series set IDs */
    SETI("id_pre",  id.pre);
    SETI("id_rlh",  id.rlh);
    SETI("id_tem",  id.tem);
    SETI("id_win",  id.win);
    SETI("id_nra",  id.nra);
    SETI("id_ira",  id.ira);
    SETI("id_lai",  id.lai);
    SETI("id_ptr",  id.ptr);
    SETI("id_pev",  id.pev);
    SETI("id_spe",  id.spe);
    SETI("id_inr",  id.inr);

#undef SETD
#undef SETI
#undef SETS
}

/* Read back writable variables the user may have changed. */
static void readback_state(PyObject *dict)
{
    PyObject *v;

#define GETINT(name, dst) \
    if ((v = PyDict_GetItemString(dict, name)) != NULL && PyLong_Check(v)) \
        dst = (int)PyLong_AsLong(v)
#define GETDBL(name, dst) \
    if ((v = PyDict_GetItemString(dict, name)) != NULL && PyFloat_Check(v)) \
        dst = PyFloat_AsDouble(v)

    GETINT("verbose",   verbose);
    GETINT("soilverb",  soilverb);
    GETDBL("thetol",    thetol);
    GETINT("maxitr",    maxitr);

#undef GETINT
#undef GETDBL
}

/* --- Built-in module methods ------------------------------------------ */

static PyObject *
py_vamps_quit(PyObject *self, PyObject *args)
{
    exit(0);
}

static PyObject *
py_vamps_print(PyObject *self, PyObject *args)
{
    const char *s;
    if (!PyArg_ParseTuple(args, "s", &s))
        return NULL;
    fprintf(stdout, "%s\n", s);
    Py_RETURN_NONE;
}

static PyObject *
py_vamps_getset_y(PyObject *self, PyObject *args)
{
    int set, pos;
    if (!PyArg_ParseTuple(args, "ii", &set, &pos))
        return NULL;
    return PyFloat_FromDouble(data[set].xy[pos].y);
}

static PyObject *
py_vamps_getset_x(PyObject *self, PyObject *args)
{
    int set, pos;
    if (!PyArg_ParseTuple(args, "ii", &set, &pos))
        return NULL;
    return PyFloat_FromDouble(data[set].xy[pos].x);
}

static PyObject *
py_vamps_getset_points(PyObject *self, PyObject *args)
{
    int set;
    if (!PyArg_ParseTuple(args, "i", &set))
        return NULL;
    return PyLong_FromLong((long)data[set].points);
}

static PyObject *
py_vamps_getset_name(PyObject *self, PyObject *args)
{
    int set;
    if (!PyArg_ParseTuple(args, "i", &set))
        return NULL;
    return PyUnicode_FromString(data[set].name);
}

static PyObject *
py_vamps_getset_fname(PyObject *self, PyObject *args)
{
    int set;
    if (!PyArg_ParseTuple(args, "i", &set))
        return NULL;
    return PyUnicode_FromString(data[set].fname);
}

static PyObject *
py_vamps_getsetbyname(PyObject *self, PyObject *args)
{
    const char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    return PyLong_FromLong((long)getsetbyname((char *)name));
}

static PyObject *
py_vamps_printsum(PyObject *self, PyObject *args)
{
    printsum(stderr);
    Py_RETURN_NONE;
}

static PyObject *
py_vamps_printstr(PyObject *self, PyObject *args)
{
    const char *name, *val;
    if (!PyArg_ParseTuple(args, "ss", &name, &val))
        return NULL;
    printstr(name, val);
    Py_RETURN_NONE;
}

static PyObject *
py_vamps_cpu(PyObject *self, PyObject *args)
{
    return PyFloat_FromDouble(cpusec());
}

/* getspar(nr, parmname) — access sp[nr] fields from Python (e.g. soilf.py) */
static PyObject *
py_vamps_getspar(PyObject *self, PyObject *args)
{
    int nr;
    const char *des;
    if (!PyArg_ParseTuple(args, "is", &nr, &des))
        return NULL;
    if (strcmp(des, "thetas") == 0)        return PyFloat_FromDouble(sp[nr].thetas);
    if (strcmp(des, "ksat") == 0)          return PyFloat_FromDouble(sp[nr].ksat);
    if (strcmp(des, "residual_water") == 0) return PyFloat_FromDouble(sp[nr].residual_water);
    if (strcmp(des, "n") == 0)             return PyFloat_FromDouble(sp[nr].n);
    if (strcmp(des, "alpha") == 0)         return PyFloat_FromDouble(sp[nr].alpha);
    if (strcmp(des, "l") == 0)             return PyFloat_FromDouble(sp[nr].l);
    if (strcmp(des, "psisat") == 0)        return PyFloat_FromDouble(sp[nr].psisat);
    if (strcmp(des, "b") == 0)             return PyFloat_FromDouble(sp[nr].b);
    return PyFloat_FromDouble(MISSVAL);
}

/* readset(fname, name) — load a time-series file and return the dataset ID */
static PyObject *
py_vamps_readset(PyObject *self, PyObject *args)
{
    const char *fname, *name;
    if (!PyArg_ParseTuple(args, "ss", &fname, &name))
        return NULL;
    get_data((char *)fname, (char *)name, -1);
    return PyLong_FromLong((long)getsetbyname((char *)name));
}

/* getdefstr(section, key, default) — read a string value from the .inp file */
static PyObject *
py_vamps_getdefstr(PyObject *self, PyObject *args)
{
    const char *section, *key, *defval;
    const char *result;
    if (!PyArg_ParseTuple(args, "sss", &section, &key, &defval))
        return NULL;
    result = getdefstr((char *)section, (char *)key, (char *)defval, infilename, FALSE);
    return PyUnicode_FromString(result ? result : defval);
}

static PyMethodDef VampsMethods[] = {
    {"quit",      py_vamps_quit,      METH_NOARGS,  "Quit VAMPS"},
    {"print",     py_vamps_print,     METH_VARARGS, "Print string to stdout"},
    {"printsum",  py_vamps_printsum,  METH_NOARGS,  "Print simulation summary to stderr"},
    {"printstr",  py_vamps_printstr,  METH_VARARGS, "Store a named string in the output file"},
    {"cpu",       py_vamps_cpu,       METH_NOARGS,  "Return CPU seconds used so far"},
    {"getset_y",       py_vamps_getset_y,      METH_VARARGS, "getset_y(set, pos) -> float"},
    {"getset_x",       py_vamps_getset_x,      METH_VARARGS, "getset_x(set, pos) -> float"},
    {"getset_points",  py_vamps_getset_points, METH_VARARGS, "getset_points(set) -> int"},
    {"getset_name",    py_vamps_getset_name,   METH_VARARGS, "getset_name(set) -> str"},
    {"getset_fname",   py_vamps_getset_fname,  METH_VARARGS, "getset_fname(set) -> str"},
    {"getsetbyname",   py_vamps_getsetbyname,  METH_VARARGS, "getsetbyname(name) -> int; -1 if not found"},
    {"getspar",        py_vamps_getspar,       METH_VARARGS, "getspar(nr, parmname) -> float; read soil parameter"},
    {"readset",        py_vamps_readset,       METH_VARARGS, "readset(fname, name) -> int; load time-series, return set ID"},
    {"getdefstr",      py_vamps_getdefstr,     METH_VARARGS, "getdefstr(section, key, default) -> str; read from .inp file"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef vampsmoduledef = {
    PyModuleDef_HEAD_INIT,
    "vamps",
    "VAMPS simulation state module",
    -1,
    VampsMethods
};

PyMODINIT_FUNC
PyInit_vamps(void)
{
    PyObject *m = PyModule_Create(&vampsmoduledef);
    if (!m)
        return NULL;
    /* State attributes are populated at runtime via update_state() */
    return m;
}

/* -----------------------------------------------------------------------
 * Callback mechanism
 * ----------------------------------------------------------------------- */

/* Call a named Python function from __main__ (no arguments).
 * Returns 0 on success, -1 on error, 1 if function not defined. */
static int
call_hook(const char *funcname)
{
    PyObject *main_mod = PyImport_AddModule("__main__");
    if (!main_mod)
        return -1;

    if (!PyObject_HasAttrString(main_mod, funcname))
        return 1; /* not defined — that's fine */

    PyObject *func = PyObject_GetAttrString(main_mod, funcname);
    if (!func || !PyCallable_Check(func)) {
        PyErr_Clear();
        Py_XDECREF(func);
        return 1;
    }

    PyObject *result = PyObject_CallNoArgs(func);
    Py_DECREF(func);
    if (!result) {
        fprintf(stderr, "vamps: error in Python %s():\n", funcname);
        PyErr_Print();
        return -1;
    }
    Py_DECREF(result);
    return 0;
}

/* Helper: get the vamps module dict, update state, call hook, read back. */
static int
run_hook(const char *funcname)
{
    PyObject *mod = PyImport_ImportModule("vamps");
    if (!mod) {
        PyErr_Print();
        return -1;
    }
    PyObject *d = PyModule_GetDict(mod); /* borrowed ref */
    update_state(d);
    Py_DECREF(mod);

    int ret = call_hook(funcname);

    /* Read back any changes the script made to writable vars */
    mod = PyImport_ImportModule("vamps");
    if (mod) {
        d = PyModule_GetDict(mod);
        readback_state(d);
        Py_DECREF(mod);
    }
    return ret < 0 ? ret : 0;
}

int py_at_start_f(void)  { return run_hook("at_start");  }
int py_each_step_f(void) { return run_hook("each_step"); }
int py_at_end_f(void)    { return run_hook("at_end");    }

/* Call Python estrs() and return its float result.
 * Returns -1.0 on error or if estrs() is not defined in Python. */
double
py_estrs_f(void)
{
    PyObject *main_mod, *func, *result;
    double ret = -1.0;

    main_mod = PyImport_AddModule("__main__");
    if (!main_mod) return ret;
    if (!PyObject_HasAttrString(main_mod, "estrs")) return ret;

    func = PyObject_GetAttrString(main_mod, "estrs");
    if (!func || !PyCallable_Check(func)) { Py_XDECREF(func); return ret; }

    /* Update state before calling */
    {
        PyObject *mod = PyImport_ImportModule("vamps");
        if (mod) {
            update_state(PyModule_GetDict(mod));
            Py_DECREF(mod);
        }
    }

    result = PyObject_CallNoArgs(func);
    Py_DECREF(func);
    if (!result) {
        fprintf(stderr, "vamps: error in Python estrs():\n");
        PyErr_Print();
        return ret;
    }
    ret = PyFloat_AsDouble(result);
    Py_DECREF(result);
    return ret;
}

/* -----------------------------------------------------------------------
 * Initialisation and cleanup
 * ----------------------------------------------------------------------- */

void
init_py(const char *script_path)
{
    /* Register the built-in "vamps" module before Py_Initialize */
    if (PyImport_AppendInittab("vamps", &PyInit_vamps) == -1) {
        fprintf(stderr, "vamps: could not register built-in Python module\n");
        exit(1);
    }

    Py_Initialize();

    /* Add vampslib to sys.path so that util.py, regress.py, etc. are
     * importable without the user needing to set $VAMPSLIB in their shell. */
    {
        char cmd[2048];
        snprintf(cmd, sizeof(cmd),
                 "import sys\n"
                 "if '%s' not in sys.path: sys.path.insert(0, '%s')\n",
                 vampslib, vampslib);
        PyRun_SimpleString(cmd);
    }

    /* Load the startup library (vamps_startup.py) from vampslib. */
    {
        char startup[2048];
        FILE *fp;
        snprintf(startup, sizeof(startup), "%s/vamps_startup.py", vampslib);
        fp = fopen(startup, "r");
        if (fp) {
            int rc = PyRun_SimpleFile(fp, startup);
            fclose(fp);
            if (rc != 0) {
                fprintf(stderr, "vamps: error in startup script: %s\n", startup);
                Py_Finalize();
                exit(1);
            }
        }
        /* Not finding vamps_startup.py is not fatal — user may not have it yet. */
    }

    /* Execute the user script in the __main__ namespace.
     * This defines at_start / each_step / at_end if the user wrote them. */
    {
        FILE *fp = fopen(script_path, "r");
        if (!fp) {
            fprintf(stderr, "vamps: cannot open Python script: %s\n", script_path);
            Py_Finalize();
            exit(1);
        }

        int rc = PyRun_SimpleFile(fp, script_path);
        fclose(fp);

        if (rc != 0) {
            fprintf(stderr, "vamps: error loading Python script: %s\n", script_path);
            Py_Finalize();
            exit(1);
        }
    }
}

void
finalize_py(void)
{
    Py_Finalize();
}

/* -----------------------------------------------------------------------
 * Soil method 5 — Python user-defined theta/h/k functions
 *
 * These C functions match the soilparmt function pointer signatures and
 * call the corresponding Python functions defined in the user script
 * (typically via "from soilf import *").
 * ----------------------------------------------------------------------- */

/* Helper: call a Python function with (int nr, double a) -> double */
static double
_py_soil_1(const char *fname, int nr, double a)
{
    PyObject *main_mod, *func, *args, *result;
    double ret = MISSVAL;

    main_mod = PyImport_AddModule("__main__");
    if (!main_mod) return ret;
    func = PyObject_GetAttrString(main_mod, fname);
    if (!func) { PyErr_Clear(); return ret; }
    args = Py_BuildValue("(id)", nr, a);
    result = PyObject_Call(func, args, NULL);
    Py_DECREF(func);
    Py_DECREF(args);
    if (result) {
        ret = PyFloat_AsDouble(result);
        Py_DECREF(result);
    } else {
        fprintf(stderr, "vamps: error in Python %s():\n", fname);
        PyErr_Print();
    }
    return ret;
}

/* Helper: call a Python function with (int nr, double a, double b) -> double */
static double
_py_soil_2(const char *fname, int nr, double a, double b)
{
    PyObject *main_mod, *func, *args, *result;
    double ret = MISSVAL;

    main_mod = PyImport_AddModule("__main__");
    if (!main_mod) return ret;
    func = PyObject_GetAttrString(main_mod, fname);
    if (!func) { PyErr_Clear(); return ret; }
    args = Py_BuildValue("(idd)", nr, a, b);
    result = PyObject_Call(func, args, NULL);
    Py_DECREF(func);
    Py_DECREF(args);
    if (result) {
        ret = PyFloat_AsDouble(result);
        Py_DECREF(result);
    } else {
        fprintf(stderr, "vamps: error in Python %s():\n", fname);
        PyErr_Print();
    }
    return ret;
}

double py_h2dmc(int nr, double head, int layer)        { return _py_soil_1("h2dmc", nr, head);        }
double py_t2k  (int nr, double wcon, int layer)        { return _py_soil_1("t2k",   nr, wcon);        }
double py_t2h  (int nr, double wcon, double depth, int layer) { return _py_soil_2("t2h", nr, wcon, depth); }
double py_h2t  (int nr, double head, int layer)        { return _py_soil_1("h2t",   nr, head);        }
double py_h2k  (int nr, double head, int layer)        { return _py_soil_1("h2k",   nr, head);        }
double py_h2u  (int nr, double head, int layer)        { return _py_soil_1("h2u",   nr, head);        }
double py_h2dkdp(int nr, double head, int layer)       { return _py_soil_1("h2dkdp",nr, head);        }

/* Call Python getspars(secname, spnr) to read soil params from .inp file */
void
py_getspars_f(const char *secname, int spnr)
{
    PyObject *main_mod, *func, *args, *result;

    main_mod = PyImport_AddModule("__main__");
    if (!main_mod) return;
    if (!PyObject_HasAttrString(main_mod, "getspars")) {
        fprintf(stderr, "vamps: Python function getspars not defined"
                " (soil method=5 requires soilf.py or equivalent)\n");
        exit(1);
    }
    func = PyObject_GetAttrString(main_mod, "getspars");
    if (!func) { PyErr_Print(); exit(1); }
    args = Py_BuildValue("(si)", secname, spnr);
    result = PyObject_Call(func, args, NULL);
    Py_DECREF(func);
    Py_DECREF(args);
    if (!result) {
        fprintf(stderr, "vamps: error in Python getspars():\n");
        PyErr_Print();
        exit(1);
    }
    Py_DECREF(result);
}

/* -----------------------------------------------------------------------
 * Scripted topsystem (Python) — pre_top / post_top / tstep_top
 *
 * Python's tstep_top(tstep) must return a 4-tuple:
 *   (precipitation, interception, transpiration, soilevaporation)
 * ----------------------------------------------------------------------- */

void
py_pre_top_script(void)
{
    run_hook("pre_top");
}

void
py_post_top_script(void)
{
    run_hook("post_top");
}

void
py_tstep_top_script(int tstep, double *precipitation, double *interception,
                    double *transpiration, double *soilevaporation)
{
    PyObject *main_mod, *func, *args, *result;
    PyObject *mod, *d;

    main_mod = PyImport_AddModule("__main__");
    if (!main_mod) return;

    /* Update state so Python sees current id_* values */
    mod = PyImport_ImportModule("vamps");
    if (mod) {
        d = PyModule_GetDict(mod);
        update_state(d);
        Py_DECREF(mod);
    }

    func = PyObject_GetAttrString(main_mod, "tstep_top");
    if (!func || !PyCallable_Check(func)) {
        fprintf(stderr, "vamps: Python function tstep_top not defined\n");
        Py_XDECREF(func);
        return;
    }

    args = Py_BuildValue("(i)", tstep);
    result = PyObject_Call(func, args, NULL);
    Py_DECREF(func);
    Py_DECREF(args);

    if (!result) {
        fprintf(stderr, "vamps: error in Python tstep_top():\n");
        PyErr_Print();
        return;
    }

    /* Unpack 4-tuple: (precipitation, interception, transpiration, soilevaporation) */
    if (!PyTuple_Check(result) || PyTuple_Size(result) != 4) {
        fprintf(stderr, "vamps: tstep_top() must return a 4-tuple "
                "(precipitation, interception, transpiration, soilevaporation)\n");
        Py_DECREF(result);
        return;
    }

    *precipitation  = PyFloat_AsDouble(PyTuple_GET_ITEM(result, 0));
    *interception   = PyFloat_AsDouble(PyTuple_GET_ITEM(result, 1));
    *transpiration  = PyFloat_AsDouble(PyTuple_GET_ITEM(result, 2));
    *soilevaporation = PyFloat_AsDouble(PyTuple_GET_ITEM(result, 3));
    Py_DECREF(result);
}

#endif /* HAVE_LIBPYTHON */
