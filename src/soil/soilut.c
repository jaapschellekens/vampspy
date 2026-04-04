/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/soilut.c,v 1.26 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: soilut.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DEBUG
static char RCSid[] =
"$Id: soilut.c,v 1.26 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "vamps.h"
#include "soils.h"
#include "swatsoil.h"
#include <math.h>

double volm1;
extern double volsat,volact;

double  noop(int nr, double head, int layer)
{
	perror("oops");
	exit(1);
	return MISSVAL;
}
double  (*h2dmc) (int nr, double head, int layer) = NULL;
double  (*t2k) (int nr, double wcon, int layer) = NULL;
double  (*t2h) (int nr, double wcon, double depth, int layer) = NULL;
double  (*h2t) (int nr, double head, int layer) = NULL;
/* in work */
double  (*h2k) (int nr, double head, int layer) = noop;
double  (*h2u) (int nr, double head, int layer) = noop;
double  (*h2dkdp) (int nr, double head, int layer) = noop;

/* this is were the Clapp Horberger (Campbel 1994) functions start */

/*C:h2dmc_0
 *@ double h2dmc_0 (int nr, double head)
 * 
 * Description: Calculates the differential moisture capacity as a
 * function of pressure head. This is the Campbel 1994
 *
 * Returns: differential moisture content*/
double
h2dmc_0 (int nr, double head, int layer)
{
	if (head >= -1.0E-1) {
		return 0.0;
	} else
	return (sp[nr].thetas/(-sp[nr].b*sp[nr].psisat))*
		pow(head/sp[nr].psisat,-(1.0/sp[nr].b)-1.0);
	/* NOTREACHED */
}

/*C:t2k_0
 *@ double t2k_0(int nr, double wcon)
 * 
 * Description: Calculation of hydraulic conductivity from water
 * content. Clapp Hornberger, or a look up table. This replaces
 * hconode()
 *
 * Returns: k_unsat*/
double
t2k_0 (int nr, double wcon, int layer)
{
	double relsat;

	relsat = (wcon - sp[nr].residual_water) / (sp[nr].thetas -
			sp[nr].residual_water);
	relsat = relsat > 1.0 ? 1.0 : relsat; 
	if (relsat < 0.001)
		return 1.0E-10;
	else{
	  return  sp[nr].ksat* pow(wcon/sp[nr].thetas,sp[nr].n*sp[nr].b);
	  /*	  return  sp[nr].ksat* pow(wcon/sp[nr].thetas,sp[nr].b*2+3); */
	}
}


/*C:t2h_0
 *@double t2h_0 (int nr, double wcon, double depth)
 *
 * Calculates the pressure heads at layer j from the moisture content
 * using Clapp/Hornberger
 * Returns: pressure head
 */

#define MAXSUCKHEAD -1.0E20
double
t2h_0 (int nr, double wcon, double depth, int layer)
{
	double ans;

	if ((sp[nr].thetas - wcon) == 0.0 /*< 1.0E-5*/)
		return fabs (depth);
	else{
		if ((wcon - sp[nr].residual_water) < 1.0E-5) {
			return MAXSUCKHEAD * (1-(wcon - sp[nr].residual_water));
		}
		ans =  sp[nr].psisat * pow(wcon/sp[nr].thetas,-1*sp[nr].b);
		return ans < MAXSUCKHEAD ? MAXSUCKHEAD * (1-(wcon -
				sp[nr].residual_water)) : ans;
	}
}

/*C:h2t_0
 *@ double h2t_0 (int nr, double head)
 * 
 * Calculate the water content at layer j from pressure
 * head @head@
 *
 * Returns: theta
 */
double
h2t_0 (int nr, double head, int layer)
{
	if (head > sp[nr].psisat)
		return sp[nr].thetas;
	else
		return sp[nr].thetas*pow(head/sp[nr].psisat,-1/sp[nr].b);
	/* NOTREACHED */
}

double
h2k_0( int nr, double head, int layer)
{
	if (head >= sp[nr].psisat)
		return sp[nr].ksat;
	else
		return sp[nr].ksat * pow(head/sp[nr].psisat,-sp[nr].n);
}

double
h2u_0 (int nr, double head, int layer)
{
	if (head >= sp[nr].psisat)
		return sp[nr].ksat * (-sp[nr].psisat/(sp[nr].n-1) + head - sp[nr].psisat);
	else
		return -(h2k_0(nr,head,layer) * head)/(sp[nr].n-1);
}

double h2dkdp_0(int nr, double head, int layer)
{
	if (head >= sp[nr].psisat)
		return 0.0;
	else
		return -(sp[nr].n * sp[nr].ksat)/sp[nr].psisat * pow(head/sp[nr].psisat,-sp[nr].n -1);
}


/* This is where the vam Genuchten functions start */

/*C:h2dmc_1
 *@ double h2dmc_1 (int nr, double head)
 * 
 * Calculates the differential moisture capacity as a function of pressure head
 *
 * Returns: differential moisture capacity*/
double
h2dmc_1 (int nr, double head, int layer)
{
	double term1, term2;
	double alphah;
	double ans;

	if (head >= -1.0E-1){
		return 0.0;
	} else {/* OK we us van Genuchten */
		/* Compute alpha *h */
		alphah = fabs (sp[nr].alpha * head);
		/* Compute|alpha *h|to power n-1 */
		term1 = pow (alphah, sp[nr].n - 1.0);
		term2 = term1 * alphah;
		/* add one and raise to the power m+1 */
		term2 = pow (1.0 + term2, sp[nr].m + 1.0);
		/* dvide theta-s minus theta-r by term2 */
		term2 = (sp[nr].thetas - sp[nr].residual_water) / term2;
		/* Calculate the diff moist coeff */
		ans = fabs (-1.0 * sp[nr].n * sp[nr].m *
				sp[nr].alpha * term2 * term1);
		if (head > -1.0 && ans < 1.0E-7)
			return 0.0;
		else
			return ans;
	} 
}

/*C:t2k_1
 *
 * Prototype: double t2k_1(int nr, double wcon)
 * 
 * Description: Calculation of hydraulic conductivity from water content. Via van Genuchten
 *
 * Returns: k_unsat */
double
t2k_1 (int nr, double wcon, int layer)
{
  double relsat, term1, expon1;

  relsat = (wcon - sp[nr].residual_water) / (sp[nr].thetas -
						sp[nr].residual_water);
  relsat = relsat > 1.0 ? 1.0 : relsat; 
  if (relsat < 0.001)
      return 1.0E-10;
  else
     {
	expon1 = sp[nr].l / sp[nr].m - 2.0;
	term1 = pow (1.0 - pow (relsat, 1.0 / sp[nr].m), sp[nr].m);
	return sp[nr].ksat * pow (relsat, expon1) *
	  (1.0 - term1) * (1.0 - term1);
     } 
}


/*C:t2h_1
 *
 * Prototype: double t2h_1 (int nr, double wcon, double depth)
 * 
 * Description: Calculates the pressure heads at layer j from the
 * water content It uses van Genuchten
 *
 * Returns: pressure head */

double
t2h_1 (int nr, double wcon, double depth, int layer)
{
  double help;

  if ((sp[nr].thetas - wcon) < 1.0E-6)
	return fabs (depth);
  else
     {			/* Calculate according to van Genuchten */
	if ((wcon - sp[nr].residual_water) < 1.0E-6)
	  {
	     return -1.0E16;
	  }
	else
	  {
	     /* First the inverse of the sorptivity is calculated */
	     help = (sp[nr].thetas - sp[nr].residual_water) /
	       (wcon - sp[nr].residual_water);
	     /* Raise to the power of 1/m */
	     help = pow (help, 1.0 / sp[nr].m);
	     /* Substract one and raise to the power of 1/n */
	     help = pow ((help - 1.0), 1.0 / sp[nr].n);
	     /* Devide by alpha */
	     return -1.0 * fabs (help / sp[nr].alpha);
	  }
     }
}


/*C:h2t_1
 *
 * Prototype: double h2t_1 (int nr, double head)
 * 
 * Description: Calculate the water content at layer j from pressure
 * head, via van Genuchten
 *
 * Returns: water content theta */
double
h2t_1 (int nr, double head, int layer)
{
  double help;

   if (head >= -1.0E-6)
     return sp[nr].thetas;
   else
     {
	help = pow (fabs (sp[nr].alpha * head), sp[nr].n);
	help = pow (1.0 + help, sp[nr].m);
	return sp[nr].residual_water + ((sp[nr].thetas -
					      sp[nr].residual_water) / help);
     }
    /* NOTREACHED */
}


double
h2k_1( int nr, double head, int layer)
{
	return 0.0;
}

double
h2u_1 (int nr, double head, int layer)
{
	return 0.0;
}

double h2dkdp_1(int nr, double head, int layer)
{
	return 0.0;
}

/*C:h2dmc_2
 *
 * Prototype: double h2dmc_2 (int nr, double head)
 * 
 * Description: Calculates the differential moisture capacity as a function of pressure head.
 *
 * Returns: differential moisture capacity */
double
h2dmc_2 (int nr, double head, int layer)
{
  if (head >= -1.0E-1)
      return 0.0;
  else
     return getval (&sp[nr].tab[H2DMCTAB], head, layer);
}

/*+Name: t2k_2
 *
 *Prototype: double t2k_2(int nr, double wcon)
 * 
 *Description: Calculation of hydraulic conductivity from water
 *content. a look up table. This replaces hconode()
 *
 * Returns: k_unsat+*/
double
t2k_2 (int nr, double wcon, int layer)
{
   double relsat;

   relsat = (wcon - sp[nr].residual_water) / (sp[nr].thetas -
						  sp[nr].residual_water);
   relsat = relsat > 1.0 ? 1.0 : relsat;
   if (relsat < 0.001)
     return 1.0E-10;
   else
     return getval (&sp[nr].tab[T2KTAB], wcon, layer);
}

/*+Name: t2h
 *Prototype: double prhnod_2e (int nr, double wcon, double depth)
 *Description: Calculates the pressure heads at layer j from the water content look-up table
 *Returns: pressure head
 +*/

double
t2h_2 (int nr, double wcon, double depth, int layer)
{
  if ((sp[nr].thetas - wcon) < 1.0E-6)
	return fabs (depth);
  else
     {
	if ((wcon - sp[nr].residual_water) < 1.0E-6)
	  return -1.0E16;
	else
	  return getval (&sp[nr].tab[T2HTAB], wcon, layer);
     }
}

/*+Name: h2t_2
 *Prototype: double h2t_2 (int nr, double head)
 * 
 *Desciption: Calculate the water content at layer j from pressure head head
 *Returns: water content (theta)
 +*/
double
h2t_2 (int nr, double head, int layer)
{
	if (head >= -1.0E-6)
		return sp[nr].thetas;
	else
		return getval (&sp[nr].tab[H2TTAB], head, layer);
}

double
h2k_2 (int nr, double head, int layer)
{
	if (head >= 0.0)
		return sp[nr].ksat;
	else
		return getval (&sp[nr].tab[H2KTAB], head, layer);
}

double
h2u_2 (int nr, double head, int layer)
{
	return getval (&sp[nr].tab[H2UTAB], head, layer);
}


double
h2dkdp_2 (int nr, double head, int layer)
{
	if (head >= 0.0)
		return 0.0;
	else
		return getval (&sp[nr].tab[H2DKDPTAB], head, layer);
}




/*C:watcon
 *@void watcon(void)
 *
 * Determines the actual water content of the profile
 */ 
void
watcon ()
{
	int i;

	volm1 = volact;
	volact = 0.0;

	for (i = 0; i < layers; i++) {
		volact -= (theta[i] * dz[i]);
	}
}

