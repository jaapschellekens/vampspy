/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/reduceva.c,v 1.16 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: reduceva.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static char RCSid[] =
"$Id: reduceva.c,v 1.16 1999/01/06 12:13:01 schj Alpha $";

#include "swatsoil.h"
#include <math.h>

double  cofred;
double  reva;
int     swredu;
extern double peva;

/*C:reduceeva
 *@double reduceva (int swreduc)
 *
 * Calculates reduction of soil evaporation
 *
 * Returns reducted soil evaporation */

double  ldwet, spev, spev1, saev, saev1, theta1, thepf2;

double
reduceva (int swreduc)
{
	double  t1,reva = 0;

	switch (swreduc){
		case 0:
			reva = peva;
			break;
		case 1:
			/* Black model */
			if (daynr == 1)
				ldwet = 0.0;
			if (prec > 1.0)
				ldwet = 0.0;
			ldwet++;
			reva = cofred * (sqrt (ldwet) - sqrt (ldwet - 1.0));
			reva = reva < peva ? reva : peva;
			break;
		case 2:
			/* Boesten and Stroosnijder */
			if (daynr == 1){
				spev = 0.0;
				saev = 0.0;
			}
			if ((prec - intc) < peva){
				spev += (peva - (prec - intc));
				t1 = spev < cofred * sqrt (spev) - saev ? 
					spev : cofred * sqrt (spev) - saev;
				reva = prec + t1;
				saev = spev < cofred * sqrt (spev) ?
					spev : cofred * sqrt (spev);
			}else{
				reva = peva;
				saev = 0.0 > saev - (prec - intc - peva) ?
					0.0 : saev - (prec - intc - peva);
				spev = saev > (saev * saev) / (cofred * cofred) ?
					saev : (saev * saev) / (cofred * cofred);
			}
			break;
		case 3:
			/* Adapted Boesten and Stroosnijder */
			if (daynr == 1){
				thepf2 = node[1].sp->h2t (node[1].soiltype, -100.0);
				spev1 = thepf2 - theta[0] * fabs (dz[0]);
				if (spev1 > 0.0)
					saev1 = cofred * sqrt (spev1);
				else
					saev1 = 0.0;
				theta1 = theta[0];
			}
			spev1 -= (theta[0] - theta1) * fabs (dz[0]);
			spev = spev1 + peva - (prec - intc);
			spev = spev < peva ? peva : spev;
			if (spev1 > 0.0)
				saev1 = cofred * sqrt (spev1);
			else
				saev1 = 0.0;
			saev = cofred * sqrt (spev);
			reva = saev - saev1 + (prec - intc);
			theta1 = theta[0];
			break;
		default:
			Perror(progname,1,0,RCSid,"Method not known","");
			break;
	}

  return reva;
}
