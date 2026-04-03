/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/integral.c,v 1.22 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: integral.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DEBUG
static  char RCSid[] =
"$Id: integral.c,v 1.22 1999/01/06 12:13:01 schj Alpha $";
#endif

#include <math.h>
#include "swatsoil.h"

double  cuminflow=0.0;
double  cumtop=0.0;
double  cumbot=0.0;
double  cumprec=0.0;
double  cumintc=0.0;
double  cumeva=0.0;
double  cumtra=0.0;
double	masbal =0.0;
double	masbal_old =0.0;
double	runots =0.0;
double	runodt =0.0;
double rootts=0.0;
double  cqdra=0.0;
double  cumdra=0.0;
double  surface_runoff = 0.0;
double	rootextract = 0.0;
extern  double volini;

double  _cumprec = 0.0;
double  _cumintc = 0.0;
double  _cumeva = 0.0;
double  _cumtra = 0.0;



/*C:intergral
 *@ void integral(int i)
 *
 * Integrate variables over timestep and calculates mass bal
 */
void
integral(int ii)
{
  double evapts,revats,epndts,tt;
  int i,j;

  _cumprec +=(prec*dt);
  _cumintc +=(intc*dt);
  _cumeva +=(reva*dt);
  _cumtra +=(ptra*dt);

  cqdra=0.0;
  for (i=0;i<layers; i++){
	  rootextract+=qrot[i]*dt;
	  rootts+=qrot[i]*dt;
	  if (dodrain){
		  for (j = 0; j < 3; j++)
			  cqdra += qdra[j][i];
	  }
  }

  revats = reva *dt;
  /* Actual soil evaporation/ ponding evaporation of this timestep */
  epndts = revats > pond ? pond : revats;
  epndts = epndts < 0.0 ? 0.0 : epndts;

  tt = revats - pond < (qtop+prec-intc)*dt ? revats -pond : (qtop+prec-intc)*dt;
  tt = tt >0.0 ? tt : 0.0;
  evapts = pond >= revats ? 0.0 :  tt;
  
  pond += (qtop + prec - intc)*dt - epndts- evapts;

  if (pond < 1.0E-6) pond =0.0;
  
  runodt=0.0;
  if (pond > pondmx){
	  runodt = pond - pondmx;
	  pond = pondmx;
  }
  runots += runodt;
  surface_runoff += runodt;	
  cumtop += (qtop * dt);
  cumbot += (qbot * dt);
  cumdra += (cqdra *dt);
  masbal_old= masbal;
  masbal = (volini + cumbot - cumdra - cumtop - rootextract) - volact;
}
