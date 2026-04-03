/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/filltab.c,v 1.12 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: filltab.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */


static char RCSid[] =
"$Id: filltab.c,v 1.12 1999/01/06 12:13:01 schj Alpha $";

#include "vamps.h"
#include "swatsoil.h"
#include   "soils.h"
#include   "nrutil.h"
#include <string.h>
#include <float.h>

int     tablesize=300;
int	dumptables=1;

/*C:mktblinter
 *@ double *mktblinter(int layer,int nrpoints,double min, double max)
 *
 * Makes an increasing theta array taking into account the slope of
 * the theta 2 k relation
 * Returns: Theta array
 */
double *mktblinter(int spnr,int nrpoints,double min, double max)
{
	XY *Ktmpxy = NULL;
	double *Slopes;
	double inter;
	double avgslope = 0.0;
	double *tt;
	int i; 

	inter = (max - min) / nrpoints;
	Ktmpxy = (XY *) ts_memory(NULL,nrpoints * sizeof(XY),progname);
	tt = (double *) ts_memory(NULL,nrpoints * sizeof(double),progname);

	for (i= 0 ; i< nrpoints; i++){
		Ktmpxy[i].x = i * inter + min;
		Ktmpxy[i].y = sp[spnr].t2k (spnr,Ktmpxy[i].x);
	}
	Slopes = ts_slopes(Ktmpxy,nrpoints);


	for (i=0;i<nrpoints; i++){
		Slopes[i] += 0.1;
		avgslope += 1.0/Slopes[i];
	}
	avgslope /= (double) nrpoints;

	tt[0] = inter / avgslope * 1/Slopes[0] + min;
	for (i=1;i<nrpoints;i++){
		tt[i] = inter / avgslope * 1/Slopes[i] + tt[i-1];
	}

	free(Ktmpxy);
	free(Slopes);
	return tt;
}

/*+Name:filltables
 *  Prototype: void filltables (int spnr,int layer,int estdmc)
 *  Description: fills the look-up tables. If estdmc = 1 then
 *  the differential moisture content is determined using ts_slopes.
 *
 *  Returns: nothing+*/
void
filltables (int spnr,int layer,int estdmc)
{
	int i;
	XY *tmpxy;double *dmc;
	double inter;
	char des[1024];
	double xtra=0.0005;
	double *thetaint;


	/* fist theta2k */
	thetaint = mktblinter(spnr,tablesize,sp[spnr].residual_water, sp[spnr].thetas + xtra);
	sp[spnr].tab = (TBL *) ts_memory (NULL, MAXTBL * sizeof (TBL), progname);
	for (i=0;i<MAXTBL; i++){
		switch (i){
			case T2KTAB:
				strcpy(des,"t2k");
				break;
			case T2HTAB:
				strcpy (des,"t2h");
				break;
			case H2DMCTAB:
				strcpy (des,"h2dmc");
				break;
			case H2TTAB:
				strcpy(des,"h2t");
				break;
			case H2UTAB:
				strcpy(des,"h2u");
				break;
			case H2KTAB:
				strcpy(des,"h2k");
				break;
			case H2DKDPTAB:
				strcpy(des,"h2dkdp");
				break;
		}
		sp[spnr].tab[i]= *mktable(tablesize,des,verbose);
	}

	for (i = 0; i < sp[spnr].tab[T2KTAB].points; i++)
	{
		sp[spnr].tab[T2KTAB].x[i] = thetaint[i];
		sp[spnr].tab[T2KTAB].y[i] =
			sp[spnr].t2k (spnr,sp[spnr].tab[THETA2K].x[i]);

		sp[spnr].tab[T2HTAB].x[i] = sp[spnr].tab[T2KTAB].x[i];
		sp[spnr].tab[T2HTAB].y[i] = sp[spnr].t2h(spnr,sp[spnr].tab[T2HTAB].x[i],depth[layer]);

		inter =sp[spnr].t2h(spnr,sp[spnr].residual_water, depth[layer])/(double)sp[spnr].tab[T2KTAB].points;
		sp[spnr].tab[H2TTAB].y[i] = sp[spnr].tab[T2KTAB].x[i];
		sp[spnr].tab[H2TTAB].x[i]= sp[spnr].tab[T2HTAB].y[i];

		if(!estdmc){
			sp[spnr].tab[H2DMCTAB].x[i]=
				sp[spnr].tab[H2TTAB].x[i];
			sp[spnr].tab[H2DMCTAB].y[i]=
			     sp[spnr].h2dmc(spnr,sp[spnr].tab[H2DMCTAB].x[i]);
		}
		sp[spnr].tab[H2KTAB].y[i] = sp[spnr].tab[T2KTAB].y[i];
		sp[spnr].tab[H2KTAB].x[i]= sp[spnr].tab[T2HTAB].y[i];
		sp[spnr].tab[H2UTAB].x[i]= sp[spnr].tab[T2HTAB].y[i];
		/* sp[spnr].tab[H2UTAB].y[i]= sp[spnr].h2u(spnr,sp[spnr].tab[T2HTAB].y[i]); */
		sp[spnr].tab[H2DKDPTAB].x[i]= sp[spnr].tab[T2HTAB].y[i];
		/*sp[spnr].tab[H2DKDPTAB].y[i]= sp[spnr].h2dkdp(spnr,sp[spnr].tab[T2HTAB].y[i]);*/
	}
	if (estdmc){
		tmpxy = (XY *) ts_memory(NULL,sp[spnr].tab[T2HTAB].points * sizeof(XY),progname);
		dmc = dvector(0,sp[spnr].tab[T2HTAB].points);
		for (i = 0; i< sp[spnr].tab[T2HTAB].points; i++){
			tmpxy[i].x = sp[spnr].tab[T2HTAB].y[i];	
			tmpxy[i].y = sp[spnr].tab[T2HTAB].x[i];	
		}
		dmc = ts_slopes(tmpxy,sp[spnr].tab[T2HTAB].points);
		for (i = 0; i< sp[spnr].tab[T2HTAB].points; i++){
			sp[spnr].tab[H2DMCTAB].x[i]=
				sp[spnr].tab[H2TTAB].x[i];
			sp[spnr].tab[H2DMCTAB].y[i]=dmc[i] > DBL_MAX ? 0.0 : dmc[i];
		}
		free(tmpxy);
		free_dvector(dmc,0,sp[spnr].tab[T2HTAB].points);
	}


  
	if (dumptables)
		for (i=0;i<MAXTBL;i++){
			printcom(RCSid);
			printcom("Dumped look-up tables follow");
			sprintf(des,"%s_%d_x",sp[spnr].tab[i].des,spnr);
			printar(des,sp[spnr].tab[i].x,sp[spnr].tab[i].points);
			sprintf(des,"%s_%d_y",sp[spnr].tab[i].des,spnr);
			printar(des,sp[spnr].tab[i].y,sp[spnr].tab[i].points);
		}
	free(thetaint);
}

