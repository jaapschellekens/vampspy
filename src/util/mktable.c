/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/mktable.c,v 1.16 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: mktable.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */
#include "vamps.h"
#include "swatsoil.h"
#include <string.h>
#include <float.h>

static char RCSid[] =
"$Id: mktable.c,v 1.16 1999/01/06 12:13:01 schj Alpha $";


TBL
* mktable (int items,char *des,int verb)
{
	TBL *tt;

	if (verb)
		fprintf (stderr, 
		"mktable.c: creating table %s with %d points\n", des, items);
	
	tt = (TBL *) ts_memory (NULL, sizeof (TBL), progname);
	tt->x = (double *) ts_memory (NULL, items * sizeof (double), progname);
	tt->y = (double *) ts_memory (NULL, items * sizeof (double), progname);
	tt->points = items;
	tt->lasthit = 0.5 * items;
	strcpy (tt->des, des);

	return tt;
}


/*C:getval
 *@double getval(TBL *tab, double x)
 *
 * Gets the y value corresponding to @x@ from @tab@. This version uses
 * the lasthit hint in the table as a starting point for a
 * hunt phase. This is followed by bisection and interpolation.
 * In speed.inp this method is about 18% faster than the one without the
 * hunt fase.
 */ 
double getval(TBL *tab, double x)
{
	register unsigned int jhi,jm,inc,index,n;
	register int ascnd;
	double top,bott,rico;

	n = tab->points; /* number of points in the table */
	ascnd = (tab->x[n -1] >= tab->x[0]); /* are we ascending? */

	if (tab->lasthit < 0 || tab->lasthit >= n){ /* hint seems _not_valid */
		tab->lasthit = -1;
		jhi = n;
	}else{					/* we have a valid hint... */
		inc = 1;
		if (x >= tab->x[tab->lasthit] == ascnd){
			if (tab->lasthit == n - 1) 
				return tab->y[n-1]; /* last point */
			jhi = tab->lasthit + 1;
			while ( x >= tab->x[jhi] == ascnd) {
				tab->lasthit = jhi;
				inc += inc;
				/*jhi = tab->lasthit + inc;
				 * because lasthit == jhi rewtite to:
				 */ 
				jhi += inc;
				if (jhi >= n){
					jhi = n;
					break;
				}
			}
		} else {
			if (tab->lasthit == 0){
				tab->lasthit = -1;
				return tab->y[0];
			}
			jhi = tab->lasthit--;
			while(x < tab->x[tab->lasthit] == ascnd) {
				jhi = tab->lasthit;
				inc <<= 1;
				if (inc >= jhi){
					tab->lasthit = -1;
					break;
				}else 
					/* tab->lasthit = jhi - inc; 
					 * becuase jhi == lastit rewrite 
					 * (this might save use sime time)
					 * to: */
					tab->lasthit -= inc;
			}
		}
	} /* hunting stopped, do bin search...*/
	while ((jhi - tab->lasthit) != 1){
		jm = (jhi + tab->lasthit) >> 1;
		if (x >= tab->x[jm] == ascnd)
			tab->lasthit = jm;
		else
			jhi = jm;
	}
	if ( x == tab->x[0]) /* at first point and match */
		tab->lasthit = 0;
	else if (x == tab->x[n - 1]) /* at last point and match */
		tab->lasthit = n - 2;

	/* linear interpolation */
	top = (tab->y[tab->lasthit + 1] - tab->y[tab->lasthit]);
	bott = (tab->x[tab->lasthit + 1] - tab->x[tab->lasthit]);
	rico =  top/bott;
	return (rico * (x - tab->x[tab->lasthit]))
		+ tab->y[tab->lasthit];
}



/*C:__getval
 *
 *@ double __getval(TBL *tab, double xval)
 * 
 * get the y value from xval from table tab. Linear interpolation
 * is used after the search. This is the old method.
 */

double __getval(TBL *tab, double xval)
{
	unsigned long index,mid,lo,up,n;
	int ascnd;
	double top,bott,rico;

  	/* first check high and low points */	
	if (xval <= tab->x[0])
		return tab->y[0];
	else if (xval >= tab->x[(tab->points) - 1])
		return tab->y[(tab->points) - 1];

	/* Find position in array: x[index] < xval < x[index +1]*/
	n = tab->points;
	lo = -1; up = n;

	ascnd = (tab->x[n - 1] >= tab->x[0]);
	while (up - lo > 1){
		mid = (up + lo) >> 1;
		if (xval >= tab->x[mid] == ascnd)
			lo = mid;
		else
			up = mid;
	}
	if ( xval == tab->x[0])  index = 0;
	else if (xval == tab->x[n-1]) index =  n - 2;
	else index = lo;

	/* linear interpolation */
	top = (tab->y[index + 1] - tab->y[index]);
	bott = (tab->x[index + 1] - tab->x[index]);
	rico =  top/bott;
	return (rico * (xval - tab->x[index])) + tab->y[index];
}

/*C_getval
 *
 *@ double _getval(dataset *ds, double xval)
 * 
 * gets the y value from xval from set DS. Linear interpolation
 * is used after the search
 */
double _getval(dataset *ds, double xval)
{
	unsigned long index,mid,lo,up,n;
	int ascnd;
	double rico;

  	/* first check high and low points */
	
	if (xval <= ds->xy[0].x)
		return ds->xy[0].y;
	else if (xval >= ds->xy[(ds->points) - 1].x)
		return ds->xy[(ds->points) - 1].y;

	/* Find position in array: x[index] < xval < x[index +1]*/
	n = ds->points;
	lo = -1; up = n;
	ascnd = (ds->xy[n - 1].x >= ds->xy[0].x);
	while (up - lo > 1){
		mid = (up + lo) >> 1;
		if (xval >= ds->xy[mid].x == ascnd)
			lo = mid;
		else
			up = mid;
	}
	if ( xval == ds->xy[0].x)  index = 0;
	else if (xval == ds->xy[n-1].x) index =  n - 2;
	else index = lo;

	if (xval == ds->xy[index].x)
		return ds->xy[index].y; /* exact match */

	/* linear interpolation */
	rico = (ds->xy[index + 1].y - ds->xy[index].y) / (ds->xy[index + 1].x - ds->xy[index].x);
	return (rico * (xval - ds->xy[index].x)) + ds->xy[index].y;
}


#ifndef LBUFF
#define LBUFF 2048
#endif
/*-
 * void readtablefile (FILE *stream,int nr, int type)
 * reads the soil tables from an external (TOPOG _SOIL) file
 * This should be an already opened stream.
 * There are some cludges when reading these files. The entries
 * var is incorrect (substract 17!!) and at present only the following
 * columns are used:
 * 1) psi [m] (converted to cm)
 * 2) skipped
 * 3) theta
 * 4) k (converted to cm)
 * 5) differention moisture cap. (d_theta/d_phi) (converted to cm)
 * The rest is skipped
 * Types are:
 *       1     TOPOG _soil file
 *       2     white space separated columns (psi theta k) diff_moist is estimated
 *       3     white space separated columns (psi theta k diff_moist)
 */
void
readtablefile(FILE *stream,int nr,int type)
{
	/* theta = y phi = x */
	char *cp;
	double *dmc;
	XY *tmpxy;
	int lread=0;
	int datalines=0;
	int entries,i;
	char buf[LBUFF];
	char ens[512];
	double tt_k,tt_theta,tt_psi,tt_dtdp,tt_dkdp,tt_u,dummy;

	/* First count the number of lines in the file */
	while ((cp = fgets (buf,LBUFF,stream)) != (char *) NULL){
		if (*cp && *cp != '#' && *cp != '&') 
			/* main body, when not in header */
			datalines++;
	}
	rewind(stream); /* rewind the file to top */

	if (verbose)
		fprintf(stderr,"Making tables with entries: %d\n",datalines);
	/* Making the tables now we know the number of lines */
	sp[nr].tab = (TBL *) ts_memory (NULL, MAXTBL * sizeof (TBL), progname);
	sp[nr].tab[T2KTAB]= *mktable(datalines,"theta2k",verbose);
	sp[nr].tab[T2HTAB]= *mktable(datalines,"theta2head",verbose);
	sp[nr].tab[H2DMCTAB]= *mktable(datalines,"head2dmc",verbose);
	sp[nr].tab[H2TTAB]= *mktable(datalines,"head2theta",verbose);
	/* new ones for the TOPOG style solution */
	sp[nr].tab[H2KTAB]= *mktable(datalines,"head2k",verbose);
	sp[nr].tab[H2UTAB]= *mktable(datalines,"head2u",verbose);
	sp[nr].tab[H2DKDPTAB]= *mktable(datalines,"head2dkdp",verbose);

	/* No actually read the contents of the file */
	while ((cp = fgets (buf,LBUFF,stream)) != (char *) NULL){
		if (*cp && *cp != '#' && *cp != '&'){ 
			/* main body, when not in header */ 
			switch (type){
				case 2:
					if (sscanf(cp,"%lf %lf %lf",&tt_psi,&tt_theta,&tt_k)!=3){
						fclose(stream);
						Perror(progname,1,1,RCSid,"Error when reading soil table.","format 2 (psi theta k)");
					}
					break;
				case 1:
					if (sscanf(cp,"%lf %lf %lf %lf %lf %lf",&tt_psi,&tt_u,&tt_theta,&tt_k,&tt_dtdp,&tt_dkdp)!=6){
						fclose(stream);
						Perror(progname,1,1,RCSid,"Error when reading soil table.","TOPOG _soil format");
					}
					tt_k *=100;
					tt_psi *= 100;
					tt_dtdp*=0.01;
					break;
				case 3:
					if (sscanf(cp,"%lf %lf %lf %lf",&tt_psi,&tt_theta,&tt_k,&tt_dtdp)!=4){
						fclose(stream);
						Perror(progname,1,1,RCSid,"Error when reading soil table.","format 3 (psi theta k diff_moist)");
					}
				default:
					break;
			}
			lread++;
			sp[nr].tab[T2KTAB].x[datalines-lread]=tt_theta;
			sp[nr].tab[T2KTAB].y[datalines-lread]=tt_k;
			sp[nr].tab[T2HTAB].x[datalines-lread]=tt_theta;
			sp[nr].tab[T2HTAB].y[datalines-lread]=tt_psi;
			sp[nr].tab[H2DMCTAB].x[datalines-lread]=tt_psi;
			sp[nr].tab[H2DMCTAB].y[datalines-lread]=tt_dtdp;
			sp[nr].tab[H2TTAB].x[datalines-lread]=tt_psi;
			sp[nr].tab[H2TTAB].y[datalines-lread]=tt_theta;
			sp[nr].tab[H2KTAB].x[datalines-lread]=tt_psi;
			sp[nr].tab[H2KTAB].y[datalines-lread]=tt_k;
			sp[nr].tab[H2UTAB].x[datalines-lread]=tt_psi;
			sp[nr].tab[H2UTAB].y[datalines-lread]=tt_u;
			sp[nr].tab[H2DKDPTAB].x[datalines-lread]=tt_psi;
			sp[nr].tab[H2DKDPTAB].y[datalines-lread]=tt_dkdp;
		}else
		if (type ==1){ /* Get information from the file header and comments of TOPOG file*/
			if (strstr (cp,"K(sat)=") != NULL) /* get k_sat*/
				sscanf(cp,"%10c %lf",ens,&sp[nr].ksat);
			if (strstr(cp,"Theta(sat)=") != NULL) /* get porosity */
				sscanf(cp,"%14c %lf",ens,&sp[nr].thetas);
			if (strstr(cp,"Theta(dry)=") != NULL) /* get residual water */
				sscanf(cp,"%14c %lf",ens,&sp[nr].residual_water);
			if (strstr(cp,"entries")!=NULL){ /* get entries for a check */
				sscanf(cp,"%14c %d",ens,&entries);
				entries -= 17;
				if (entries != datalines)
					Perror(progname,0,0,RCSid,"Problem in TOPOG soil table:","datalines != entries this datafile might be corrupt!");
			}
		}
	}

	if (type == 2){
		tmpxy = (XY *) ts_memory(NULL,sp[nr].tab[T2HTAB].points * sizeof(XY),progname);
		dmc = (double *) ts_memory(NULL,sp[nr].tab[T2HTAB].points * sizeof(double),progname);
		for (i = 0; i< sp[nr].tab[T2HTAB].points; i++){
			tmpxy[i].x = sp[nr].tab[T2HTAB].y[i];	
			tmpxy[i].y = sp[nr].tab[T2HTAB].x[i];	
		}
		dmc = ts_slopes(tmpxy,sp[nr].tab[T2HTAB].points);
		for (i = 0; i< sp[nr].tab[T2HTAB].points; i++)
			sp[nr].tab[H2DMCTAB].y[i]=dmc[i] > DBL_MAX ? 0.0 : dmc[i];
		free(tmpxy);
		free(dmc);
	}

	if (dumptables)
		for (i=0;i<4;i++){
			printcom(RCSid);
			printcom("Dumped look-up tables follow");
			sprintf(buf,"%s_%d_x",sp[nr].tab[i].des,nr);
			printar(buf,sp[nr].tab[i].x,sp[nr].tab[i].points);
			sprintf(buf,"%s_%d_y",sp[nr].tab[i].des,nr);
			printar(buf,sp[nr].tab[i].y,sp[nr].tab[i].points);
		}
}
