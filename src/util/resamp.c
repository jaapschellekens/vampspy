/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/resamp.c,v 1.6 1999/01/06 12:13:01 schj Alpha $ */
/* $RCSfile: resamp.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static  char RCSid[] =
"$Id: resamp.c,v 1.6 1999/01/06 12:13:01 schj Alpha $";

#include "vamps.h"
#include <stdlib.h>

/*-
 * void resamp_a_to_a(XY *a, XY *b, int apoints, int bpoints)
 *
 * resamples a to b using the spline stuff from ts (see ts_spl.c)
 *
 */

XY
*resamp_a_to_b(XY *a, XY *b,int apoints,int bpoints)
{
	double *Slopes;
	XY *tmpxy;
	int i;

	if (a[0].x > b[0].x || a[apoints -1].x < b[bpoints -1].x){
      		Perror (progname,1,0, RCSid, "Resampling failed due to extrapolation", "Check input sets");
	}
	/* First calculate slopes */
	Slopes = ts_slopes (a,apoints);

	/* We need a temp structure here*/
	tmpxy = (XY *)ts_memory(NULL,bpoints * sizeof(XY),progname);

	for (i=0;i<bpoints; i++)
		tmpxy[i].x=b[i].x;

	ts_meval (tmpxy,a,Slopes,apoints,bpoints);
	free(a); /* don't need this anymore */

	/*
	for (i=0;i<bpoints; i++){
		fprintf(stderr,"%f %f %f\n",tmpxy[i].x,tmpxy[i].y,b[i].y);
	}
	*/

	free(Slopes);
	return tmpxy;
}
