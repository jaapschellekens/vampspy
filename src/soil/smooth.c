/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/smooth.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ 
 */
/*  $RCSfile: smooth.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DDEBUG
static char RCSid[] =
"$Id: smooth.c,v 1.7 1999/01/06 12:13:01 schj Alpha $";

#endif

#include "swatsoil.h"

/*+Name: smooth
 *
 *  Prototype:  void smooth (int inter,int what)
 *  Description:  smooths the ksat profile by using a simple moving
 *  average of size inter. If inter = 0 no smoothing is done.  What
 *  determine what to smooth: 1 = ksat, 2 = thetas, 3 =
 *  residual_water
 *  Remarks: Disabled at the moment!
 *  Returns: nothing+*/
void
smooth (int inter, int what)
{
  int     j, i, pt;
  double  tot;
  showit("swatsoil",ERR,"Smoothing disabled in this version",0,0);
  return;

  if (inter)
    {
      for (i = 0; i <= (inter / 2); i++)	/*
						 * Smooth first part that falls
						 * outside limit of moving average
						 */
	{
	  tot = 0.0;
	  pt = 0;
	  for (j = i; j <= (inter / 2); j++)
	    {
	      switch (what)
		{
		case 1:
		  tot += node[j].sp->ksat;
		  break;
		case 2:
		  tot += node[j].sp->thetas;
		  break;
		case 3:
		  tot += node[j].sp->residual_water;
		  break;
		}
	      pt++;
	    }
	  switch (what)
	    {
	    case 1:
	      node[i].sp->ksat = (double) (1.0 / (double) pt) * tot;
	      break;
	    case 2:
	      node[i].sp->thetas = (double) (1.0 / (double) pt) * tot;
	      break;
	    case 3:
	      node[i].sp->residual_water = (double) (1.0 / (double) pt) * tot;
	      break;
	    }
	}
      /*
       * Smooth inner part 
       */
      for (i = (inter / 2); i < layers - (inter / 2); i++)
	{
	  tot = 0.0;
	  pt = 0;
	  for (j = i - (inter / 2); j <= i + (inter / 2); j++)
	    {
	      switch (what)
		{
		case 1:
		  tot += node[j].sp->ksat;
		  break;
		case 2:
		  tot += node[j].sp->thetas;
		  break;
		case 3:
		  tot += node[j].sp->residual_water;
		  break;
		}
	      pt++;
	    }

	  switch (what)
	    {
	    case 1:
	      node[i].sp->ksat = (double) (1.0 / (double) pt) * tot;
	      break;
	    case 2:
	      node[i].sp->thetas = (double) (1.0 / (double) pt) * tot;
	      break;
	    case 3:
	      node[i].sp->residual_water = (double) (1.0 / (double) pt) * tot;
	      break;
	    }
	}
      for (i = layers - (inter / 2); i < layers; i++)	
      /* Smooth last part that falls outside limit of moving average */
	{
	  tot = 0.0;
	  pt = 0;
	  for (j = i; j < layers; j++)
	    {
	      switch (what)
		{
		case 1:
		  tot += node[j].sp->ksat;
		  break;
		case 2:
		  tot += node[j].sp->thetas;
		  break;
		case 3:
		  tot += node[j].sp->residual_water;
		  break;
		}
	      pt++;
	    }
	  switch (what)
	    {
	    case 1:
	      node[i].sp->ksat = (double) (1.0 / (double) pt) * tot;
	      break;
	    case 2:
	      node[i].sp->thetas = (double) (1.0 / (double) pt) * tot;
	      break;
	    case 3:
	      node[i].sp->residual_water = (double) (1.0 / (double) pt) * tot;
	      break;
	    }
	}
    }
}
