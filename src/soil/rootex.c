/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/rootex.c,v 1.16 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: rootex.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static char RCSid[] =
"$Id: rootex.c,v 1.16 1999/01/06 12:13:01 schj Alpha $";

#include "vamps.h"
#include "swatsoil.h"
#include <math.h>

double	drootz;
double	tronae,tronab,zronam;
int	swupfu; /* roo ex method */
extern	double	ptra; /* Potential tranporation */
double	cofszb,cofsza;
int	swsink;
double	hlim3,hlim3h,hlim3l;
double  hlim4;
int	swhypr;
double	cofsha,cofshb;
double	hlim1,hlim2u,hlim2l;
double	redosm;
double  *rootfrac; /* Fraction of roots/total soil */


/*+Name: rootex
 *
 *  Prototype: void rootex(int pt, double drz)
 *
 *  Description:  Calculates root extraction rates as a function of
 *  head pt -> timestep drz -> rooting dept in timestep
 *
 *  Returns:  nothing+*/
void
rootex (int pt,double drz)
{
  int     i,noddrz=0;
  double  t1;
  double  htotal,hlim2,rootdepth,zrona,a,b,top,bot;

  for (i = 0; i < layers; i++)	/* Set to zero */
    qrot[i] = 0.0;

  if (drz > -1.0E-3)
    {
      /* nothing here, no roots, skip */
    }
  else
    {
      /* determine lowest compartment containing roots */
      rootdepth = 0.0;		/* depth in swap */
      for (i = 0; i < layers; i++)
	{
	  noddrz = i;
	  rootdepth += dz[i];
	  if (drz >= (rootdepth - 1.0E-6))
	    break;
	}

      /* Compute zone where roots are non-active */
      zrona = 0.0;
      if (t < tronae)
	{
	  if (t > tronab)
	    zrona = zronam * (t - tronab) / (tronae - tronab);
	}
      else
	zrona = zronam;

      /* Calculate intercept and slope */
      switch (swupfu)
	{
	  /* Calculate intercept and slope (Fedded, Kowalik, Zaradny) */
	case 0:
	  a = ptra / fabs (drz);
	  b = 0.0;
	  break;
	case 1:		/* (Hoogland, Belmans, Feddes) */
	  b = cofszb;
	  t1 = ptra / fabs (drz) + 0.5 * b * fabs (drz);
	  a = cofsza < t1 ? cofsza : t1;
	  break;
	case 2:		/* (Prasad) */
	  a = ptra * 2.0 / fabs (drz);
	  b = ptra * 2.0 / pow (fabs (drz), 2.0);
	  break;
	case 3:         /* JS method, no reduction */
	  break;
	default:
	  a=0;b=0;
	  Perror(progname,1,0,RCSid,"Rootextract method not Known","");
	  break;
	}

      /* Calculation of root extraction of the compartments */
      for (i = 0; i <= noddrz; i++)
	{			/* Check this !! */
	  top = fabs (z[i] - 0.5 * dz[i]);
	  bot = fabs (z[i] + 0.5 * dz[i]);
	  if (zrona <= (z[i] + 0.5 * dz[i]))
	    bot = top;
	  else if (zrona < (z[i] - 0.5 * dz[i]))
	    bot = fabs (zrona);
	  if ((z[i] + 0.5 * dz[i]) < drz)
	    bot = fabs (drz);
	  if (swupfu == 3){
	  qrot[i] =  rootfrac[i] * ptra;
	  }else{
	  qrot[i] = (a - (b * (bot + top) * 0.2)) * (bot - top);
	  }
	}

      /* Calculation of critical point hlim3 according to Feddes
     in the Hoogland option hlim3 is given */
      if (swsink != 1)
	{
	  hlim3 = hlim3h;
	  if (ptra >= 0.1)
	    {
	      if (ptra <= 0.5)
		{
		  hlim3 = hlim3h + ((0.5 - ptra) / (0.5 - 0.1)) *
		    (hlim3l - hlim3h);
		}
	    }
	  else
	    {
	      hlim3 = hlim3l;
	    }
	  /* coefficients for exponential relation between hlim3 + hlim4 */
	  if (swhypr == 1)
	    {
	      cofshb = -1.0 * log (1.0E-2) / (hlim3 - hlim4);
	      cofsha = 1.0 / exp (cofshb * hlim3);
	    }
	}
      /* Calculation of transpiration reduction */
      hlim2 = hlim2u;
      for (i = 0; i <= noddrz; i++){
	  /* removed some solute stuff *
	   * hosma = osmota + cml[i] * osmotb;*/
	  htotal = h[i]/* + redosm * hosma*/;
	  if (i > 0)
	    hlim2 = hlim2l;
	  /* Transpiration reduction due to wet circumstances */
	  if (h[i] <= hlim1 && h[i] > hlim2)
	    qrot[i] *= (hlim1 - h[i]) / (hlim1 - hlim2);
	  /* Transpiration reduction due to dry circumstances */
	  if (h[i] <= hlim3 && htotal >= hlim4)
	    {
	      if (swhypr == 1)
		{
		  qrot[i] *= cofsha * exp (cofshb * htotal);
		}
	      else
		{
		  qrot[i] *= (hlim4 - htotal) / (hlim4 - hlim3);
		}
	    }
	  /* No transpiration */
	  if (h[i] > hlim1 || htotal < hlim4)
	    qrot[i] = 0.0;
	}
    }

}
