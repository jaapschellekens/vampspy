#ifndef PY_INIT_H_
#define PY_INIT_H_

#ifdef HAVE_LIBPYTHON
extern void   init_py(const char *script_path);
extern void   finalize_py(void);
extern int    py_at_start_f(void);
extern int    py_each_step_f(void);
extern int    py_at_end_f(void);
extern double py_estrs_f(void);

/* Soil method 5 — Python user-defined theta/h/k functions */
extern double py_h2dmc  (int nr, double head, int layer);
extern double py_t2k    (int nr, double wcon, int layer);
extern double py_t2h    (int nr, double wcon, double depth, int layer);
extern double py_h2t    (int nr, double head, int layer);
extern double py_h2k    (int nr, double head, int layer);
extern double py_h2u    (int nr, double head, int layer);
extern double py_h2dkdp (int nr, double head, int layer);
extern void   py_getspars_f(const char *secname, int spnr);

/* Scripted topsystem */
extern void   py_pre_top_script(void);
extern void   py_post_top_script(void);
extern void   py_tstep_top_script(int tstep, double *precipitation,
                    double *interception, double *transpiration,
                    double *soilevaporation);
#endif /* HAVE_LIBPYTHON */

#endif /* PY_INIT_H_ */
