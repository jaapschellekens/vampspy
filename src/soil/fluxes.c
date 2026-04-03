/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/fluxes.c,v 1.15 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: fluxes.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DDEBUG
static  char RCSid[] =
"$Id: fluxes.c,v 1.15 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "swatsoil.h"
double  cqbotts=0.0;

/*C:fluxes
 *@void fluxes(void)
 *  Determines the fluxes between the nodes and total bottom flux.
 *  Returns: nothing*/
void
fluxes()
{
    int	i;

    /* Calculation of fluxes from changes in volume compartment */
    q[0] = qtop * dt;
    inq[0] += q[0];
    for (i=0; i < layers; i++){
	q[i+1] = ((thetm1[i] - theta[i])* dz[i]) + q[i] 
	    + ((qrot[i] +  qdra[0][i] + qdra[1][i] + qdra[2][i] + 
		qdra[3][i]) * dt);
	inq[i+1] = inq [i+1] + q[i+1];
    }
    cqbotts += q[layers];
    cqbot += q[layers]; 
}
