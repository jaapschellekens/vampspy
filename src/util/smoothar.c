/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/smoothar.c,v 1.6 1999/01/06 12:13:01 schj Alpha $ */
/* $RCSfile: smoothar.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static  char RCSid[] =
"$Id: smoothar.c,v 1.6 1999/01/06 12:13:01 schj Alpha $";

#include "vamps.h"

/*-
 * double *smoothar (double *ar,int inter,int len)
 *
 * Smooths the array *ar of length len by using a simple moving
 * average of size inter. If inter = 0 no smoothing is done.
 */
double
*smoothar(double *ar,int inter,int len)
{
  int	j,i,pt;
  double tot;

  if (inter >0){
    for (i=(inter/2); i < len-(inter/2); i++){
      tot =0.0;
      pt=0;
      for (j=i - (inter/2) ;j <= i+(inter/2); j++){
	tot += ar[j];
	pt++;
      }
      ar[i] = (double)(1.0/(double)pt) * tot; 
    }
  }else{
    if (inter < 0)
      Perror(progname,1,0,RCSid,"Negative smoothing factor","What did you do?");
    else
      showit("smoothar",WARN,"Smoothar called with 0. No smoothing",1,verbose);
  }

  return ar;
}
