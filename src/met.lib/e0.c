/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/e0.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: e0.c,v $
 * $Author: schj $
 * $Date: 1999/01/06 12:13:01 $ */

/* met.h Header file for assorted meteorological functions to estimate
 * evapo(trans)piration.  Most have been
 * translated from Pascal (provided by Arnoud F) to C
 *
 * Jaap Schellekens */

#include <math.h>
#include <float.h>
#include "met.h"

/*C:e0
 *@ e0(double Rnetopen, double Slope, double Gamma, double earo)
 *
 * Calculates penman open water evaporation using net radiation.
 * Use other functions to get Gamma, Slope,  and earo and net radiation
 * above open water
 *
 * Returns: e0 in mm/day */
double
e0(double Rnetopen, double Slope, double Gamma, double earo)
{

  	Slope = Slope * 0.10; 	/* from mbar to kPa*/
  	Gamma = Gamma * 0.10;  

  	return (Slope * Rnetopen + Gamma * earo) / (Slope + Gamma);
}
