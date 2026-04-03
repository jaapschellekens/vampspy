/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/setzero.c,v 1.16 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: setzero.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DDEBUG
static  char RCSid[] =
"$Id: setzero.c,v 1.16 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "vamps.h"
#include "swatsoil.h"

extern double masbal_old;

/*C:setzero
 *@  void setzero(void)
 *
 * sets intemediate totals to zero
 *
 * Returns:  nothing */
void
setzero()
{
	int	i;

	cqdra= 0.0;
	for (i = 0; i <layers; i++){
		qrot[i] = 0.0;
		inq[i] = 0.0;
		if (dodrain){
			/* mqdra[i] = 0.0;*/
			mqdra[i] = 0.0;
		}
	}
	inq[i] = 0.0;
	cqbotts=0.0;
	rootts=0.0;
	runots=0.0;
}


extern double cuminflow;
void
settotzero()
{
	cuminflow=0.0;
	cumtop=0.0;
	cumbot=0.0;
	_cumprec=0.0;
	_cumintc=0.0;
	_cumeva=0.0;
	_cumtra=0.0;
	cumprec=0.0;
	cumintc=0.0;
	cumeva=0.0;
	cumtra=0.0;
	masbal =0.0;
	masbal_old =0.0;
	runots =0.0;
	rootts=0.0;
	cqdra=0.0;
	cumdra=0.0;
	volsat=0.0;
	surface_runoff = 0.0;
	rootextract = 0.0;
	nr_tri = 0; /* number of calls to h_by_tridiag*/
	nr_band = 0; /* number of calls to h_by_banddiag*/
	nr_sat = 0; /* number of calls to h_sat*/
	nr_itter = 0; /* total number of ittereations  */
}
