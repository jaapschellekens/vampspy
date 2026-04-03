/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/penmon.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: penmon.c,v $
 * $Author: schj $
 * $Date: 1999/01/06 12:13:01 $ */

/* metut.c Assorted meteorological functions to estimate
 * evapo(trans)piration.  Most have been
 * translated from Pascal (provided by Arnoud F) to C
 *
 * Jaap Schellekens */

#include <math.h>
#include <float.h>
#include "met.h"

/*C:penmon
 *@ double penmon(double A, double delta, double gamma, double rho,
 *@	      double c_p, double e_sat, double e_act, double r_a,
 *@	      double r_s)
 *
 * Calculates Penman-Montheith ET
 *
 *	Where:
 *		A	Available energy [W/m2]
 *		delta	Slope of the temp-vapour pressure relationship
 *			at temp T [mb/K]
 *		gamma	psychrometic constant [mb/K]
 *		rho	density of dry air [kg/m3]
 *		e_act	[mb]
 *		e_sat   [mb]
 *		r_s	[s/m]
 *		r_a	[s/m]
 *
 * Returns output in watt/m2. Devide by L (lambda) and multiply by
 * TOCM to get cm/day
 *			
 */

double
penmon (double A,double delta,double gamma,double rho,double c_p,
		double e_sat,double e_act,double r_a,double r_s)
{
	double t;
	double n;

	if (A <=0)
		return 0.0;

	t = (delta * A) + (rho * c_p * (e_sat - e_act) / r_a);
	n = delta + (gamma * (1 + (r_s / r_a)));
	return (t / n) > 0.0? (t / n) : 0.0;
}

/*C:penmon_soil
 *@ double SET(double A, double delta, double gamma)
 *
 *
 * penmons_soil - Simplified Penman mointheith ET for forest floor
 *	Where:
 *		A	Available energy [W/m2]
 *		delta	Slope of the temp-valour pressure relationship
 *			at temp T [mb/K]
 *		gamma	psychrometic constant [mb/K]
 *		output in watt/m2
 *			
 */

double
penmon_soil (double A,double delta,double gamma)
{
	double t;
	double n;

	if (A <=0)
		return 0.0;

	t = (delta * A);
	n = delta + gamma;
	return (t / n);
}
