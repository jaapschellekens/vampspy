/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/ra.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: ra.c,v $
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

/*C:ra
 *@double ra (double z, double z0, double d, double u)
 *
 * Calculates ra (aerodynamic resistance) according to:
 *@	ra = [ln((z-d)/z0)]^2 /(k^2u)
 *@	Where:
 *@		d  = displacement height
 *@		z  = height of the wind speed measurements
 *@		u  = windspeed 
 *@		z0 = roughness length
 *@		k  = von Karman constant
 *
 * Returns: ra */
double 
ra (double z, double z0, double d, double u)
{
	double k2 = 0.1681;		/* (von Karman constant)^2 */
	double tt;

	tt = log ((z - d) / z0);

	return ((tt * tt) / k2)/u;
}
