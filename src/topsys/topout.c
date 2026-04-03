/* $Header: /home/schj/src/vamps_0.99g/src/topsys/RCS/topout.c,v 1.4 1999/01/06 12:13:01 schj Alpha $ */
/* 
 *  $RCSfile: topout.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

#ifdef DEBUG
static char RCSid[] =
"$Id: topout.c,v 1.4 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "vamps.h"
#define  GENOUT "%.7f "


/*-
 * void topout(int tstep)
 * 	ini type output per timestep
 */
void
topout (int tstep)
{
	/*
  printfl("ra",canop[0].ra);
  printfl("rs",canop[0].rs);
  printfl("rho",canop[0].rho);
  printfl("ea",canop[0].ea);
  printfl("es",canop[0].es);
  printfl("gamma",canop[0].gamma);
  printfl("slope",canop[0].slope);
  printfl("VPD",VPD);
  printfl("L",canop[0].L);
  printfl("Cp",canop[0].Cp);
  printfl("interception",canop[0].E_wet);
  printfl("transpiration",canop[0].E_dry);
  printfl("Cstorage",canop[0].actstorage);
  printfl("stemflow",canop[0].stemflow);
  printfl("throughfall",canop[0].throughfall);
	*/
  printfl("interception",data[id.inr].xy[tstep].y);
  printfl("transpiration",data[id.ptr].xy[tstep].y);
  printfl("precipitation",data[id.pre].xy[tstep].y);
  printfl("soilevaporation",data[id.spe].xy[tstep].y);
  printfl("stemflow",data[id.stf].xy[tstep].y);
}
