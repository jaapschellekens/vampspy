/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/earo.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: earo.c,v $
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

/*C:earo
 *@double earo(double ea, double es, double u)
 *
 * Determines Eearo (Aerodynamic evaporation or drying power of air).
 * See calder1990284 for details.
 * @ea@ and @es@ should be in mbar, u  mean daily windspeed at 2m in m/s
 *
 * Returns earodynamic term in mm/day
 */
double 
earo(double ea, double es, double u)
{
	/* 0.1 is to convert mbar to KPa */
	return 2.6 * 0.1 * (es - ea) * (1 + 0.537 * u);
}
