/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/vslope.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: vslope.c,v $
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

/*C:vslope
 *@double vslope(double Td,double es);
 *
 * Calculates the slope of the saturation vapour pressure curve from es
 * (vapour pressure at saturation) and dry bulb temp for use in Penman
 * Eo and Penman-Montheith.  Required input: Td (dry bulb temp [oC]), es
 * (vapour pressure at saturation) [mbar] 
 *
 * Returns: slope of the vapour pressure curve
 */
double 
vslope(double Td,double es)
{
	double tkev;

	tkev = Td + 273.15;     /* T in  Kelvin */

	/* return Slope in mb */
  	return  (es * (10.79574 / 0.4343 * (273.15 / (tkev * tkev)) -
	5.028 / (tkev) + 1.50475e-4 * 8.2969 / (0.4343 * 0.4343) /
	    273.15 * exp(log(10.0) * 8.2969 * (1 - (tkev) / 273.15)) +
	0.42873e-3 * 4.76955 / (0.4343 * 0.4343) * 273.15 / (tkev * tkev) *
	  exp(log(10.0) * 4.76955 * (1 - 273.15 / (tkev)))));
}
