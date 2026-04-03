#ifndef SOILS_H
#define SOILS_H
#include "vamps.h"

typedef struct {
#ifdef MPORE
	double porefrac; /* Macropores as a fraction of total porosity */
	double k_mpore_h; /* k_mpore horizontal */
	double k_mpore_v; /* k_mpore vertical */
	double c_mpore_h; /*conducting pores horiz;fraction of total mpores */
	double c_mpore_v; /*conducting pores vert;fraction of total mpores */
#endif
	int method; /* theta/h/k relation equations (or table) */
	TBL  *tab;  /* look up table with h, dmc, k, t, u, dkdp */
	char description[512];
	int mktable; /* convert to lookuptable */
	double thetas; /* theta at saturation (porosity) */
	double ksat;   /* saturated conductivity [cm2/day] */
	double kh_kv; /* ksat_horizontal/ksat_vertical [-] */
	double n; /* van Genuchten or Campbell/clapp hornberger */
	double alpha; /* Van genuchten */
	double alphaD; /* Van genuchten for drying */ /* Not used at moment */
	double alphaW; /* Van genuchten for wetting */
	double l; /*  van Genuchten mostly 0.5 */
	double m; /* Hyst Van genuchten */
	double residual_water; /* [-] */
	double b; /*clapp & hornberger */
	double psisat; /*clapp & hornberger */
	double clayfrac;
	double  (*h2dmc) (int nr, double head);  
	double  (*t2k) (int nr, double wcon);  
	double  (*t2h) (int nr, double wcon, double depth);  
	double  (*h2t) (int nr, double head);
	double  (*h2k) (int nr, double head);  
	double  (*h2u) (int nr, double head);  
	double  (*h2dkdp) (int nr, double head);  
} soilparmt;


/* Type for soil nodes in new setup */
typedef struct {
	soilparmt *sp;
	int soiltype; /* index in array of sp struc */
	char soilsection[512];
	double thickness;
	double theta_initial;
	double h_initial;
	int as_above; /* same as above layer */
	int rlay; /* Physical -- real -- layer */
} node_t;
#endif /* SOILS_H */
