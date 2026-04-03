/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/calcgwl.c,v 1.15 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: calcgwl.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DEBUG
static  char RCSid[] =
"$Id: calcgwl.c,v 1.15 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "swatsoil.h"

/*+Name:calcgwl
 *  Prototype: void calcgwl(void)
 *  Description: calculates ground water table and perched gw table
 *  Returns: nothing
 *  Remarks: should be cleaned and updated to hold unlimited amount of gw tables
+*/
void
calcgwl(void)
{
  int	i,flsat,lastnod;
  gwl[0] = MISSVAL;
  gwl[1] = MISSVAL;
  basegw[1] = MISSVAL;
  flsat = FALSE;
  lastnod = layers;

  for (i = layers-2; i >= 0 ; i--){
    if (h[i] < 0.0) {
      /* This calculates gwl using a linear, assumes linear */
      gwl[0] = z[i+1] + h[i+1];
      gwl[0] = gwl[0] <0.0 ? gwl[0] : 0.0;
      break;
    }
  }
  /* Complete saturation */
  if (i <= 0)
    gwl[0] = 0.0; 
  
  /* Search for perched gw table NOT CHECKED YET!!*/
  for ( i= 0 ; i<layers; i++){
    if (h[i] >= 0.0) {
      gwl[1] = z[i] + h[i] < 0.0 ? z[i] + h[i] : 0.0;
      lastnod = i;
      break;
    }
  }

  flsat = TRUE;
  /* find base of perched table */
  for (i= lastnod ;i <layers; i++){
    if (flsat && h[i] < 0.0) {
      basegw[1] = z[i] - dz[i]/2;
      flsat = FALSE;
    }
    if (basegw[1] > 0.0) gwl[1] = MISSVAL;
  }
}
