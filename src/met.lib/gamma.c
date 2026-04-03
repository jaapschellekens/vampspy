/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/gamma.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: gamma.c,v $
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

/*C:mgamma
 *@double mgamma(double td, double lambda)
 *
 * Calculates the psychometric constant (@[mbarC]@) It assumes
 * an air pressure of 998mb. Cp is estimated at 1005.0 J/kgK.
 * Returns: psychrometric constant. See Waterloo 1994 for details.
 */
double 
mgamma(double td, double lambda)
{
	double p, cp, tkev;
	
  	p = 998.0;  		/* pressure in mb */
  	cp = 1005.0;   		/* J/(kgK) */
  	tkev = td + 273.15;	/* T in degrees Kelvin */
	
  	return  (cp * p)/(0.622 * lambda);   /* mbC*/
}
