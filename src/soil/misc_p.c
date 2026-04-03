/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/misc_p.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: misc_p.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DEBUG
static  char RCSid[] =
"$Id: misc_p.c,v 1.7 1999/01/06 12:13:01 schj Alpha $";
#endif

#include <math.h>
#include "swatsoil.h"

/*C:dethowsat
 *@ void dethowsat
 *
 *  Description: Determines relative saturation
 *
 *  Returns: nothing */
void
dethowsat()
{
	register int	i;
	
	for (i=0; i < layers; i++)
		howsat[i]=1-(node[i].sp->thetas-theta[i]);
}

/*C:detavgthet
 *@ double detavgtheta(int layr)
 * 
 * Description: determine average theta until (not including) layer layr
 *
 * Returns: weighted average theta */
double
detavgtheta(int layr)
{
	register int i;
	double ttavgtheta=0.0;
   
	for (i=0; i < layr && i< layers; i++)
		ttavgtheta += fabs(dz[i]/dsoilp)*theta[i];

	return ttavgtheta;
}

/*C:smd
 *@  double smd(double drz,doubel fieldh)
 * 
 *  calculates the soil moisture deficit until depth drz
 *  (usually rooting depth) with fieldcapacity at fieldh
 *
 *  Returns:  soil moisture deficit */

double 
smd(double drz, double fieldh)
{
	register int i=0;
	double ttsmd=0;
	double thetadf=0;

	while (i < layers && fabs(z[i]) < fabs(drz)){
		thetadf = node[i].sp->h2t(node[i].soiltype,fieldh) - theta[i];
		if (thetadf > 0.0)
			ttsmd -= thetadf * dz[i];
		i++;
	}

	return ttsmd;
}
