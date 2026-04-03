/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/timestep.c,v 1.24 1999/01/06 12:13:01 schj Alpha $ */

/*  $RCSfile: timestep.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DEBUG
static char RCSid[] =
"$Id: timestep.c,v 1.24 1999/01/06 12:13:01 schj Alpha $";
#endif


#include "swatsoil.h"
#include <math.h>
#include <float.h>

/* Timestep at saturation */
double dtsat = 0.1;
double thetol = 0.00001;
/*C:thetol
 *@ double thetol 
 *
 * Maximum allowed change of theta at this dt, default = 0.00001
 */

double dtmax = 0.1; 
/*C:dtmax
 *@ double dtmax 
 * 
 * Description: maximum allowed dt, default = 0.1
 */

double dtmin = 0.000001; 
/*C:dtmin 
 *@double tmin
 * Description: minimum allowed dt
 */

double dt = 1.0E-2; 
/*C:dt
 *@double dt
 * Description: current dt
 */

double dtm1 = 1.0E-2;
/*C:dtm1
 *@ double dtm1 
 * Description: previous dt
 */

double t=0.0; 
/*C:t
 *@ double t 
 *
 * Description: current time
 */

double tm1=0.0;
/*C:tm1
 *@double tm1
 * Description: previous time
 */
double tm_mult = 5.0; 

static double dtheta;
static double mdtheta;
long int nrcalls=0; /* Number of calls to the timestep procedure */

void reset_timestep(void) {
    dt    = 1.0E-2;
    dtm1  = 1.0E-2;
    t     = 0.0;
    tm1   = 0.0;
    nrcalls = 0;
}

/*C:timestep
 *@ double timestep(double t, double e_t, double *dt, double *dtm1)
 *
 *  Description: Calculation of timestep (@dt@) depending on theta
 *  changes and actual time (t). Step is synchronized to @e_t@ (end
 *  of timestep.
 *
 *  Return: new timestep (dt)*/
double
timestep (double t, double e_t, double *dt, double *dtm1)
{
	register int i;

	/* Count number of calls */
	nrcalls++;
   
	/* Store previous values */
	*dtm1 = *dt;

	if ((volsat - volact) <= DBL_EPSILON)
		*dt = dtsat;
	else{
		/* dt as determined by water transport */
		*dt = dtmax;
		
		mdtheta = fabs (thetm1[0] - theta[0]);
		for (i = 0; i < layers; i++){
			dtheta = fabs (thetm1[i] - theta[i]);
			mdtheta = mdtheta < dtheta ? dtheta : mdtheta;
		}
    
		if (mdtheta < DBL_EPSILON)
			*dt = dtmax;
		else
			*dt = tm_mult * thetol * (*dtm1) / mdtheta;
		
		/* Limit to dtmin and dtmax */
    		*dt = *dt >= dtmin ? *dt : dtmin;
		*dt = *dt <= dtmax ? *dt : dtmax;
	}
  
	/* Testing on time-record in precip ts so dt might be smaller than
	 * dtmin after all. */
	if (t + *dt - e_t >= 0.0){
		*dt = e_t - t;
		flendd= TRUE;
	}

	return *dt;
}
