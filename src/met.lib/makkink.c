/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/makkink.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: makkink.c,v $
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


/*C:makkink
 *@double makkink(double rad,double Slope,double Gamma,double Lambda)
 *
 * calculates reference evaporation according to Makkink
 * The C1 constant is taken as 0.65 and C2 0.0
 *
 * Returns: makkink reference evaporation in mm/day*/
double makkink(double rad,double Slope,double Gamma,double Lambda)
{
	double C1 = 0.65;
	double C2 = 0.0;
	/* 0.1 to calculate mbar to kpa */
	/* 1000 to calculate m to mm */

 	return  (C2 +  (C1 * (Slope)/(Slope + Gamma) * rad)) * 86400/Lambda;
}
