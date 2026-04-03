#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

#ifndef NR_LIST
#define NR_LIST
#endif

#include <stdio.h>

/* logging functions from log.c */
typedef struct {
	FILE *fl;
	int  verb;
} l_fltype;

extern void l_setprefix (char *pref);
extern int l_switch;
extern int l_verb;
extern l_fltype l_fl[];
extern void l_def_switch(int arg); /* default switch func, does nothing */
extern void (* l_switchto)(int );/* function to run in stead of exiting to 
				  system when an error function is called.
				  Only if l_switch is non zero */
extern int l_addfn(int verb, char *fn);
extern int l_addf(int verb, FILE *fp);
extern void l_closeall();
extern void l_perror(char *name);
extern void l_werror(char *name);
extern void l_error(int syserror,char *name,const char *fmt, ...);
extern int l_printf(int verb,const char *fmt, ...);

/* Growing stucture with info on all stuff added with one of 
 * the functions */
#ifdef NR_LIST
#define NR_DM 1  /* double matrix */
#define NR_FM 2  /* float matrix */
#define NR_IM 3  /* integer matrix */
#define NR_DV 4  /* double vector */
#define NR_FV 5  /* float vector */
#define NR_IV 6  /* integer vector */
#define NR_CV 7  /* character vector */
#define NR_LV 8  /* long vector */
#define NR_EMPTY -1 /* Vacant position */

typedef struct {
	short int t; /* type, see NR_?? defines above */
	unsigned int fr;
	unsigned int fc;
	unsigned int lr;
	unsigned int lc;
	unsigned int points; /* rows */
	unsigned int cols; /* cols */
	char  *desc; /* descriptive name */
	union{
		double *dm;
		float  *fm;
		int *im;
		double *dv;
		float  *fv;
		int    *iv;
		unsigned char *cv;
		unsigned long *lv;
	}o; /* data, not pointer to rows */
	union{
		double **dm;
		float  **fm;
		int **im;
		double *dv;
		float  *fv;
		int    *iv;
		unsigned char *cv;
		unsigned long *lv;
	}d;
} mdata_type;

/* growing array of structures with info on all added with one
 * of these routines */
extern mdata_type *md;
extern int md_nr; /* number of structures */

extern int nr_free(int , void *);
extern void nr_free_all(void);
extern void nr_free_substr(char *substr);
extern int nr_descr(char *desc, void *a);
#endif /* NR_LIST */

#define NR_END 0
#define FREE_ARG void*

#ifdef NR_MEM_DEBUG
extern void prmeminfo();
#endif

#ifdef NR_SQR
static float sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)
#endif

#ifdef NR_DSQR
static double dsqrarg;
#define DSQR(a) ((dsqrarg=(a)) == 0.0 ? 0.0 : dsqrarg*dsqrarg)
#endif

#ifdef NR_DMAX
static double dmaxarg1,dmaxarg2;
#define DMAX(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) > (dmaxarg2) ?\
        (dmaxarg1) : (dmaxarg2))
#endif

#ifdef NR_DMIN
static double dminarg1,dminarg2;
#define DMIN(a,b) (dminarg1=(a),dminarg2=(b),(dminarg1) < (dminarg2) ?\
        (dminarg1) : (dminarg2))
#endif

#ifdef NR_FMAX
static float maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))
#endif

#ifdef NR_FMIN
static float minarg1,minarg2;
#define FMIN(a,b) (minarg1=(a),minarg2=(b),(minarg1) < (minarg2) ?\
        (minarg1) : (minarg2))
#endif

#ifdef NR_LMAX
static long lmaxarg1,lmaxarg2;
#define LMAX(a,b) (lmaxarg1=(a),lmaxarg2=(b),(lmaxarg1) > (lmaxarg2) ?\
        (lmaxarg1) : (lmaxarg2))
#endif

#ifdef NR_LMIN
static long lminarg1,lminarg2;
#define LMIN(a,b) (lminarg1=(a),lminarg2=(b),(lminarg1) < (lminarg2) ?\
        (lminarg1) : (lminarg2))
#endif

#ifdef NR_IMAX
static int imaxarg1,imaxarg2;
#define IMAX(a,b) (imaxarg1=(a),imaxarg2=(b),(imaxarg1) > (imaxarg2) ?\
        (imaxarg1) : (imaxarg2))
#endif

#ifdef NR_IMIN
static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))
#endif

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

/* standard nrutils stuff */
extern float *vector(long nl, long nh);
extern int *ivector(long nl, long nh);
extern unsigned char *cvector(long nl, long nh);
extern unsigned long *lvector(long nl, long nh);
extern double *dvector(long nl, long nh);
extern float **matrix(long nrl, long nrh, long ncl, long nch);
extern double **dmatrix(long nrl, long nrh, long ncl, long nch);
extern int **imatrix(long nrl, long nrh, long ncl, long nch);
extern float **submatrix(float **a, long oldrl, long oldrh, long oldcl, long oldch,long newrl, long newcl);
extern float **convert_matrix(float *a, long nrl, long nrh, long ncl, long nch);
extern double **convert_dmatrix(double *a, long nrl, long nrh, long ncl, long nch);
extern void free_vector(float *v, long nl, long nh);
extern void free_ivector(int *v, long nl, long nh);
extern void free_cvector(unsigned char *v, long nl, long nh);
extern void free_lvector(unsigned long *v, long nl, long nh);
extern void free_dvector(double *v, long nl, long nh);
extern void free_matrix(float **m, long nrl, long nrh, long ncl, long nch);
extern void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);
extern void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch);
extern void free_submatrix(float **b, long nrl, long nrh, long ncl, long nch);
extern void free_convert_matrix(float **b, long nrl, long nrh, long ncl, long nch);

/* read/write stuff */
extern double **nr_dmread (char *name, int *nr, int *nc);
extern int nr_fmwrite(char *fname, float **m, int fr, int lr, int fc, int lc);
extern int nr_dmwrite(char *fname, double **m, int fr, int lr, int fc, int lc);
extern int nr_imwrite(char *fname, int **m, int fr, int lr, int fc, int lc);
extern float **nr_fmread (char *name, int *nr, int *nc);
extern int nr_fvwrite(char *fname, float *v, int fr, int lr);
extern int nr_dvwrite(char *fname, double *v, int fr, int lr);
extern int nr_ivwrite(char *fname, int *v, int fr, int lr);
extern int nr_lvwrite(char *fname,unsigned  long *v, int fr, int lr);
extern int nr_cvwrite(char *fname,unsigned  char *v, int fr, int lr);
extern int nr_genw(char *fname, void *obj);

/* basic statistics */
#ifdef NR_LIST
extern double *nr_mean(void *dat);
extern double *nr_rmsq(void *dat);
#endif

/* Utils */
extern int get_id(void *dat);
extern int get_id_byname(char *name);
extern double *todvec(void *mat,int col);
extern float *tofvec(void *mat,int col);
extern int *toivec(void *mat,int col);
extern unsigned long *tolvec(void *mat,int col);
extern unsigned char *tocvec(void *mat,int col);
extern double **todxy(void *mat,int x, int y);
extern double nr_getval(double **tab, double xval, int xcol, int ycol, int ys, int ye, int lin);
#endif /* _NR_UTILS_H_ */
