/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/lambda.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: lambda.c,v $
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

/*C:lambda
 *@double lambda(double Td)
 *
 * Calculates lambda [J/kg] from dry bulb temperature using:
 *@ 	lambda = 4185.5 * (751.0 - (0.5655 * Td))
 * Returns lambda @[J/kg]@
 */
double
lambda (double Td)
{
	return  4185.5 * (751.0 - (0.5655 * Td));
}
