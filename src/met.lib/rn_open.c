/* $Header: /home/schj/src/vamps_0.99g/src/met.lib/RCS/rn_open.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: rn_open.c,v $
 * $Author: schj $
 * $Date: 1999/01/06 12:13:01 $ */

#include <math.h>
#include "met.h"

/*C:rnet_open
 *@ double rnet_open(double Rs, double Rsout double Rnet,  double Lambda)
 *
 * Calculated net radiation over open water from measured
 * incoming and reflected solar radiadion and net long wave radiation
 * At the site.
 * Albedo of open water = 0.05. Rs, Rsout and Rnet should me W/m2
 * Returns net radiation above open water in mm/day
 */ 
double rnet_open(double Rs, double Rsout, double Rnet , double lambda)
{
	double r, Rl;

  	r = 0.05;   	/* R=0.05 is open water albedo */
  	Rl = Rs - Rnet - Rsout;
  	return (Rs * (1 - r) - Rl) * 86400 / lambda;
}

/*C:rnet_open_nN
 *@ double rnet_open_nN(double Rs, double nN, double td,
 *@		double ea, double Lambda)
 *
 * Calculated net radiation over open water from measured
 * incoming solar radiadion, td [oC], ea[mbar] and nN. Albedo
 * of open water = 0.06
 * Rs should be W/m2
 * Returns net radiation above open water in mm/day.
 * This is the original function derived by penman.
 */ 
double rnet_open_nN(double Rs, double nN, double td,double ea,  double Lambda)
{
	double sigma, T4,r,Rl;

	ea *= 0.1;  /*Van mbar naar kPa*/

  	sigma = 5.67e-8;   /*Sigma is Stefan-Boltzman constant (W/m2K4)*/
  	r = 0.06;   	/* R=0.06 is open water albedo */
	T4 = exp(log(td + 273.15) * 4);
	Rl = 86400 * sigma * T4 * (0.56 - 0.248 * sqrt(ea)) *
		(0.1 + 0.9 * nN) / Lambda;
  	/*Rl is the long wave radiation component*/
  	/*86400 is sec. a day*/
  	return  Rs * 86400 * (1 - r) / Lambda - Rl;  /*Rs as mean daily W/m2*/
}
