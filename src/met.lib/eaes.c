/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/eaes.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: eaes.c,v $
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

/*C:eaes
 *@void  eaes(double td,double rh,double *ea, double *es)
 *
 * Determines saturation vapour pressure (@ea@) and actual vapour
 * presssure (@ea@) from relative humidity and dry bulb temperature. @es@
 * is calculated according to Bringfeld 1986. Relative humidity should
 * be in % and dry bulb temp in degrees Celcius. @ea@ and @es@
 * are in mbar*/
void
eaes(double td,double rh,double *ea, double *es)
{
	double tkel;

	tkel = td + 273.15;     /* T in degrees Kelvin*/

	/* es according to Bringfelt 1986 */
        *es = exp(log(10.0) * (10.79574 * (1 - 273.15 / (tkel)) -
          5.028 * log((tkel) / 273.15) / log(10.0) +
          1.50475e-4 * (1 - exp(log(10.0) * -8.2969 *
                  ((tkel) / 273.15 - 1))) + 0.42873e-3 * exp(
              log(10.0) * 4.76955 * (1 - 273.15 / (tkel))) + 0.78614));

	/* now calculate ea from rh in % */
	*ea = rh * (*es) / 100.0;
}
