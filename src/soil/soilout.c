/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/soilout.c,v 1.30 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: soilout.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DEBUG
static char RCSid[] =
"$Id: soilout.c,v 1.30 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "vamps.h"
#include "swatsoil.h"
#define  GENOUT "%.4f "
extern double volini;

/*+Name: soilout
 *
 *  Prototype:  void soilout(int tstep)
 *
 *  Description:  ini type output per timestep
 *
 *  Returns:  nothing +*/
void
soilout (int tstep)
{
  printfl ("dt", dt);
  printfl ("pond", pond);
  printfl ("surface_runoff", surface_runoff);
  printfl ("runots", runots);
  printfl ("cumeva", cumeva);
  printfl ("cumtra", cumtra);
  printfl ("cumprec", cumprec);
  printfl ("cumintc", cumintc);
  printfl ("cqbot", cqbot);
  printfl ("cqbotts", cqbotts);
  printfl ("cumtop", cumtop);
  printfl ("qtop", qtop);
  printfl ("qbot", qbot);
  printfl ("cumbot", cumbot);
  printfl ("avgtheta", avgtheta);
  printfl ("SMD", SMD);   
  printfl ("rootextract", rootextract);
  printfl ("rootts", rootts/thiststep);
  printfl ("soilevaporation", reva);
  printfl ("pot_soilevaporation", peva);
  printfl ("prec", prec);
  printfl ("intc", intc);
  printfl ("masbal", masbal);
  printfl ("volact", volact);
  printfl ("ptra", ptra);
  printar ("theta", theta, layers);
  printar ("k", k, layers);
  printar ("h", h, layers);
  printar ("gwl", gwl, 2);
  printar ("q", q, layers + 1);
  printar ("inq", inq, layers + 1);
  printar ("qrot", qrot, layers);
  printar ("howsat", howsat, layers);
  if (dodrain){
	  printfl("cqdra",cqdra);
	  printfl("cumdra",cumdra);
	  printar("drainage",mqdra,layers);
  }
  printint("converror",error[tstep]);
  printint("itter",itter[tstep]);
}
