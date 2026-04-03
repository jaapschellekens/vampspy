/* $Header: /home/schj/src/vamps_0.99g/src/topsys/RCS/pre_can.c,v 1.4 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: pre_can.c,v $
 * $Author: schj $
 * $Date: 1999/01/06 12:13:01 $ */
#include "topsys.h"
#include "vamps.h"
#include "deffile.h"

/* */
void
tstep_top_pre_canop(int tstep, double *precipitation, double *interception,
		double *transpiration, double *soilevaporation)
{
	*precipitation = data[id.pre].xy[tstep].y;
	*interception = data[id.inr].xy[tstep].y;
	*transpiration = data[id.ptr].xy[tstep].y;
	*soilevaporation = data[id.spe].xy[tstep].y;
}


void
pre_top_pre_canop()
{
	/* read ptr, spe and inr sets if these are not already determined...*/

	/* read transpiration...*/
	(void)get_data(getdefstr("ts","ptr","NON",infilename,TRUE),"ptr",0);
	id.ptr = getsetbyname("ptr");
	/* read soil evaporation...*/
	(void)get_data(getdefstr("ts","spe","NON",infilename,TRUE),"spe",0);
	id.spe = getsetbyname("spe");
	/* read interception...*/
	(void)get_data(getdefstr("ts","inr","NON",infilename,TRUE),"inr",0);
	id.inr = getsetbyname("inr");
}

void
post_top_pre_canop()
{
}
