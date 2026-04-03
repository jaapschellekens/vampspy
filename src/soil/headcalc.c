/*$Header: /home/schj/src/vamps_0.99g/src/soil/RCS/headcalc.c,v 1.35 1999/01/06 12:13:01 schj Alpha $ 
 */
/*  $RCSfile: headcalc.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */
#define MBALERR 0.5E-3 /* default max error in mass balance */

#define TREDU 2.0 /* initial used for reducing timestep */

#ifdef DEBUG
static char RCSid[] =
"$Id: headcalc.c,v 1.35 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "swatsoil.h"
#include "marquard.h"
#include "nrutil.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

/*
 * These are scratch arrays of layers size, allocated in mkscratch(),
 * freed in freescratch()
 */
static double *greekb, *greekc;
static double *thomfhold, *hold, *capold, *thoma, *thomb, *thomc, *thomf, *theold;
static double *km1,*kgeomm1,*dmcm1;

int mbck = 0; /* if set to 1 convergence is checked via mass-balance */
double mbalerr = MBALERR;
int noit = 0;
long int nr_tri = 0; /* number of calls to h_by_tridiag*/
long int nr_hitt = 0; /* number of calls to h_itt*/
long int nr_band = 0; /* number of calls to h_by_banddiag*/
long int nr_sat = 0; /* number of calls to h_sat*/
long int nr_itter = 0; /* total number of ittereations  */
int	minitr = 0;
extern double thetol;
extern int *error;
extern int *itter;
double **mat;
double **a1;
int *indx;
int     numeq, ngwl;
static int wassat = 0;

int solvemet = TRI; 

/*C:ckcnv
 *@int (*ckcnv)() = NULL;
 *
 * Pointer to function that checks for convergence. At the
 * moment 2 are present, th_ckcnv (swattr method) and mb_ckcnv
 * (a new experimental method). The pointer is set in @getparm.c@.
 * */
int (*ckcnv)() = NULL;

int mb_ckcnv()
{
	register int i;
	static double v = 0.0, vm1 = 0.0;

	vm1 = v;
	v = 0.0;

	q[0] = qtop * dt;
	for (i=0; i < layers; i++){
		q[i+1] = ((thetm1[i] - theta[i])* dz[i]) + q[i] 
			+ ((qrot[i] +  qdra[0][i] + qdra[1][i] +
					qdra[2][i] + qdra[3][i]) * dt);
		v -= (theta[i] * dz[i]);
	}

	if (fabs(q[layers] - (v - vm1) - (qtop*dt)) > mbalerr)
		return 1;
	else
		return 0;
}

/*C:th_ckcnv
 *@int th_ckcnv()
 * 
 * original swatr method to check convergence
 * */
int th_ckcnv()
{
#ifdef UNROLL_TEST
	register int i,j;
	register double  t1,t2,t3,t4;

	j = layers % 4;
	for (i = 0; i < j; i++){
		t1 = fabs (theold[i] - theta[i]);
		if (t1 > (thetol)||(h[0] > (pond - depth[0])))
			return 1;
	}
	for (i = j; i < layers; i+=4){
		t1 = fabs (theold[i] - theta[i]);
		t2 = fabs (theold[i+1] - theta[i+1]);
		t3 = fabs (theold[i+2] - theta[i+2]);
		t4 = fabs (theold[i+3] - theta[i+3]);
		if (t1 > (thetol)||(h[0] > (pond - depth[0])))
			return 1;
		else if (t2 > (thetol)||(h[0] > (pond - depth[0])))
			return 1;
		else if (t3 > (thetol)||(h[0] > (pond - depth[0])))
			return 1;
		else if (t4 > (thetol)||(h[0] > (pond - depth[0])))
			return 1;
	}
	return 0;
#else
	register int i;
	register double  t1;

	for (i = 0; i < layers; i++){
		t1 = fabs (theold[i] - theta[i]);
		if (t1 > (thetol)||(h[0] > (pond - depth[0])))
			return 1;
	}
	return 0;
#endif
}

/*C:headcalc
 *@void headcalc(int pt, double *t)
 *
 * This is the new revamped headcalc. It first checks to see if the
 * profile is completely saturated. If so a trick from @swat91@ is
 * used (@h_sat()@) the change to profile to almost saturated.
 * Unsaturated flow is first solved via a tridiagonal matrix
 * solution. In some cases this fails and a more general (band
 * diagonal) solution is used (See Press et al) which is somewhat
 * slower but more robust
 *
 * Returns: 1 if convergence failed, 0 if success */
int
headcalc (int pt, double *t)
{
#ifndef HAVE_MEMCPY
	int j;
#endif
	int i;
	int numbit = 0;
	int noconv = 1;
	int nonoit;
	int  cksatu ();
	double tredu = TREDU;
	void mat_cof_up ();
	void mat_cof_det ();
	void savevars();
	void h_satu();
	void h_itt();
	int h_by_tridiag();
	void h_by_banddiag();
	
	nonoit = noit;
	
	/* First we need to store some values regarding the present state  */
#ifdef HAVE_MEMCPY	
	memcpy((double *)hm1,(double *)h,layers * sizeof(double));
	memcpy((double *)thetm1,(double *)theta,layers * sizeof(double));
	memcpy((double *)km1,(double *)k,layers * sizeof(double));
	memcpy((double *)dmcm1,(double *)diffmoist,layers * sizeof(double));
	memcpy((double *)kgeomm1,(double *)kgeom,layers + 1 * sizeof(double));
#else	
	for (j = 0; j < layers; j++){
		hm1[j] = h[j];
		thetm1[j] = theta[j];
		km1[j] = k[j];
		dmcm1[j] = diffmoist[j];
		kgeomm1[j + 1] = kgeom[j + 1];
	}
#endif	

	if (lbc == 0)
		numeq = ngwl;
	else
		numeq = layers - 1;

	/* Reduce timestep until convergence reached */
	while(numbit == 0 || (dt > (tredu * dtmin) && noconv == 1)){
		itter[pt]++;
		numbit++;
		nr_itter++;
		mat_cof_det ();
		savevars();
		if ((wassat = cksatu())){
			h_satu();
			nonoit=1;
		}else {
			nonoit=noit;
			if (solvemet == GEN)
				h_itt();
			else{
				if (solvemet == BAN)
					h_by_banddiag();
				else
					if (h_by_tridiag())
						h_by_banddiag();
			}
		}

		/* calculate rest from new head */
		theta[numeq] = node[numeq].sp->
			h2t (node[numeq].soiltype, h[numeq]);
		diffmoist[numeq] = node[numeq].sp->
			h2dmc (node[numeq].soiltype, h[numeq]);
		for (i = numeq - 1; i >= 0; i--){
			theta[i] = node[i].sp->h2t (node[i].soiltype, h[i]);
			diffmoist[i] = node[i].sp->
				h2dmc (node[i].soiltype, h[i]);
		}

		if (nonoit == 1)
		  break;

		/* Try a few iterations before reducing the timestep */
		while(((noconv = ckcnv()) == 1 && numbit < maxitr)
				|| numbit < minitr){
			itter[pt]++;
			qtop = bocotop (&kgeom[0],&ftoph);
			mat_cof_up ();
			savevars();
			if ((wassat = cksatu())){
				h_satu();
				nonoit=1;
			}else {
				nonoit=noit;
				if (solvemet == GEN)
					h_itt();
				else{
					if (solvemet == BAN)
						h_by_banddiag();
					else	
						if (h_by_tridiag())
							h_by_banddiag();
				}
			}

			/* calculate rest from new head */
			theta[numeq] = node[numeq].sp->
				h2t (node[numeq].soiltype, h[numeq]);
			diffmoist[numeq] = node[numeq].sp->
				h2dmc (node[numeq].soiltype, h[numeq]);
			for (i = numeq - 1; i >= 0; i--){
				theta[i] = node[i].sp->
					h2t(node[i].soiltype, h[i]);
				diffmoist[i] = node[i].sp->
					h2dmc (node[i].soiltype, h[i]);
			}
			numbit++;
		} 
		if (noconv){/*start again with smaller timestep */
			dt /= tredu;
			if (flendd)
				flendd = FALSE;
			/* Restore old values */
#ifdef HAVE_MEMCPY			
			memcpy((double *)hm1,(double *)h,layers * 
					sizeof(double));
			memcpy((double *)thetm1,(double *)theta,layers * 
					sizeof(double));
			memcpy((double *)km1,(double *)k,layers * 
					sizeof(double));
			memcpy((double *)dmcm1,(double *)diffmoist,layers * 
					sizeof(double));
			memcpy((double *)kgeomm1,(double *)kgeom,layers + 1 * 
					sizeof(double));
#else
			for (j = 0; j < layers; j++){
				h[j] = hm1[j];
				theta[j] = thetm1[j];
				k[j] = km1[j];
				diffmoist[j] = dmcm1[j];
				kgeom[j + 1] = kgeomm1[j + 1];
			}
#endif			
			qtop = bocotop (&kgeom[0],&ftoph);
			numbit = 0;
		}
	}

	/* Do something here in future! */
	if (noconv){
		/*
		raise(SIGALRM);
		showit("headcalc",WARN,"no convergence, try to reduce dtmin and thetol",2,verbose);
		*/
	}
	/* Add timestep to time */
	*t += dt;

	/* Determine unknown boundary fluxes */
	if (!wassat && ftoph)
		qtop = -kgeom[0] * ((h[0] - pond) / depth[0] + 1);
	else if (wassat && lbc == 5) 
		qtop = qtop < -(volsat - volact + cqdra) ?
			-(volsat - volact + cqdra): qtop; 

	if (lbc == 4)
		qbot = -kgeom[layers - 1] * ((h[layers - 1] - 
				h[layers - 1 - 1])/depth[layers - 1] + 1);
	else if (lbc == 0||lbc == 5)
		qbot = 0.0;
	else if (lbc == 1)
		qbot = _getval(&data[id.qbo],*t);

	/* Adjust conductivities  */
	k[0] = node[0].sp->t2k (node[0].soiltype, theta[0]);
	for (i = 1; i < layers; i++){
		k[i] = node[i].sp->t2k (node[i].soiltype, theta[i]);
		kgeom[i] = MKKGEOM(i);
	}
	kgeom[layers] = k[layers - 1];
	wassat = 0;

	return noconv;
}





/*C:mkscratch
 * @void mkscratch()
 * Allocated the scratch arrays needed in @headcalc()@. They are freed
 * in @freescratch@.
 */ 
void
mkscratch ()
{
	greekb = dvector(0, layers-1);
	greekc = dvector(0, layers-1);
	thoma = dvector(0, layers-1);
	thomb = dvector(0, layers-1);
	thomc = dvector(0, layers-1);
	thomf = dvector(0, layers-1);
	thomfhold = dvector(0, layers-1);
	theold = dvector(0, layers-1);
	capold = dvector(0, layers-1);
	hold = dvector(0, layers-1);
	km1 = dvector(0, layers-1);
	kgeomm1 = dvector(0,layers);
	dmcm1 = dvector(0,layers-1);
	mat = dmatrix (1, layers, 1, 4);
	a1 = dmatrix (1, layers ,1,  4);
	indx = ivector(1,layers);
}

/*C:freescratch
 *@ void freescratch()
 * Frees the scratch arrays used in @headcalc()@. This is called from 
 * @postsoil()@
 */ 
void
freescratch ()
{
	free_dvector (greekb,0,layers-1);
	free_dvector (greekc,0,layers-1);
	free_dvector (thoma,0,layers-1);
	free_dvector (thomb,0,layers-1);
	free_dvector (thomc,0,layers-1);
	free_dvector (thomf,0,layers-1);
	free_dvector (thomfhold,0,layers-1);
	free_dvector (theold,0,layers-1);
	free_dvector (capold,0,layers-1);
	free_dvector (hold,0,layers-1);
	free_dvector (km1,0,layers-1);
	free_dvector (kgeomm1,0,layers);
	free_dvector (dmcm1,0,layers-1);
	free_ivector (indx,1,layers);
	free_dmatrix(mat,1,layers,1,4);
	free_dmatrix(a1,1,layers,1,4);
}


/*C:cksatu
 *@ int cksatu()
 * Checks if the profile is completely saturated.
 * Returns: 1 if completely saturated, otherwise 0*/
int 
cksatu ()
{
#ifndef UNROLL_TEST
	int     i;

	for (i = 0; i < layers; i++)
		if (h[i] < 0.0)
			return 0;

	return 1;
#else
	int     i,j;

	j = layers % 4;
	for( i = 0; i < j; i++)
		if (h[i] < 0.0)
			return 0;

	for (i = j; i < layers; i+=4){
		if (h[i] < 0.0)
			return 0;
		else if (h[i+1] < 0.0)
			return 0;
		else if (h[i+2] < 0.0)
			return 0;
		else if (h[i+3] < 0.0)
			return 0;
	}

	return 1;
#endif
}


/*+Name: mat_cof_set
 * Prototype: void mat_cof_det()
 * Description: Determines the coefficients a,b,c of the tridiagonal
 *  matrix 
 * Returns: nothing+*/
void
mat_cof_det ()
{
	int     i;
	double  dtdz;

	/* Calculation of coefficients for layer 0 */

	if (ftoph){	/*h at soil surface prescribed */
		thomc[0] = -dt * kgeom[1] / dz[0] / depth[1];
		thomb[0] = -thomc[0] + diffmoist[0] + dt * kgeom[0] /
			depth[0] / dz[0];
		thomf[0] = diffmoist[0] * h[0] + dt / (-dz[0]) * 
			(kgeom[0] - kgeom[1] - qrot[0] - qdra[0][0] - 
			 qdra[1][0] - qdra[2][0] - qdra[3][0])
			+ dt * kgeom[0] * pond / depth[0] / dz[0];
	}else{/* q at soil surface specified */
		thomc[0] = -dt * kgeom[1] / dz[0] / depth[1];
		thomb[0] = -thomc[0] + diffmoist[0];
		thomf[0] = diffmoist[0] * h[0] + dt / (-dz[0]) * 
			(-qtop - kgeom[1] - qrot[0] - qdra[0][0] -
			 qdra[1][0] - qdra[2][0] - qdra[3][0]);
	}

	/* Calculation of coefficients for interior layers */
	for (i = 1; i < numeq; i++){
		dtdz = -dt/dz[i];
		thoma[i] = dtdz * kgeom[i] / depth[i];
		thomc[i] = dtdz * kgeom[i + 1] / depth[i + 1];
		thomb[i] = -thoma[i] - thomc[i] + diffmoist[i];
		thomf[i] = diffmoist[i] * h[i] + dtdz *
			(kgeom[i] - kgeom[i + 1] -
			 qrot[i] - qdra[0][i] - qdra[1][i] - qdra[2][i] -
			 qdra[3][i]);
	}

	/* Calculation of coefficients for last layer */
	if (lbc == 0){
		thoma[numeq] = 0.0;
		thomc[numeq] = 0.0;
		thomb[numeq] = 1.0;
		thomf[numeq] = h[numeq];
	}else{
		if (lbc == 4){
			thoma[numeq] = 0.0;
			thomb[numeq] = 1.0;
			thomf[numeq] = h[numeq];
		}else{
			thoma[numeq] = -dt * kgeom[numeq] / dz[numeq] /
				depth[numeq];
			thomb[numeq] = -thoma[numeq] + diffmoist[numeq];
			thomf[numeq] = diffmoist[numeq] * h[numeq] +
				dt / (-dz[numeq]) * (kgeom[numeq] + qbot -
				qrot[numeq] - qdra[0][numeq] - qdra[1][numeq] -
				qdra[2][numeq] - qdra[3][numeq]);
		}
	}
}

/*C:mat_cof_up
 *@ void mat_cof_up(void)
 * 
 * Description: Updated the coeffcients b,f of the tridiagonal
 *  matrix. This is used in the iteration step.  
 *
 * Returns: nothing
 */
void
mat_cof_up ()
{
	int     i;
#ifdef UNROLL_TEST
	int j;
#endif

	if (ftoph){/* Head controlled boundary */
		thomb[0] = -thomc[0] + diffmoist[0] + dt * kgeom[0] / depth[0]
			/ dz[0];
	thomf[0] = -theta[0] + theold[0] + diffmoist[0] * h[0] + dt / 
		(-dz[0]) * (kgeom[0] - kgeom[1] - qrot[0] - qdra[0][0] - 
		qdra[1][0] - qdra[2][0] - qdra[3][0]) + dt * 
		kgeom[0] * pond / depth[0] / dz[0];
	}else{/* We have a flux controlled boundary */
		thomb[0] = -thomc[0] + diffmoist[0];
		thomf[0] = -theta[0] + theold[0] + diffmoist[0] * h[0] + dt
			/ (-dz[0]) * (-qtop - kgeom[1] - qrot[0] - 
					qdra[0][0] - qdra[1][0] - qdra[2][0] -
					qdra[3][0]);
	}

#ifdef UNROLL_TEST
	/* rest of layers */
	j = (numeq+1) % 4;
	for (i=1; i <j; i++){
		thomb[i] = thomb[i] - capold[i] + diffmoist[i];
		thomf[i] = thomf[i] - capold[i] * hold[i] + diffmoist[i] *
			h[i] - theta[i] + theold[i];
	}
	for (i = j; i <= numeq; i+=4){
		thomb[i] = thomb[i] - capold[i] + diffmoist[i];
		thomf[i] = thomf[i] - capold[i] * hold[i] + diffmoist[i] *
			h[i] - theta[i] + theold[i];
		thomb[i+1] = thomb[i+1] - capold[i+1] + diffmoist[i+1];
		thomf[i+1] = thomf[i+1] - capold[i+1] * hold[i+1] + diffmoist[i+1] *
			h[i+1] - theta[i+1] + theold[i+1];
		thomb[i+2] = thomb[i+2] - capold[i+2] + diffmoist[i+2];
		thomf[i+2] = thomf[i+2] - capold[i+2] * hold[i+2] + diffmoist[i+2] *
			h[i+2] - theta[i+2] + theold[i+2];
		thomb[i+3] = thomb[i+3] - capold[i+3] + diffmoist[i+3];
		thomf[i+3] = thomf[i+3] - capold[i+3] * hold[i+3] + diffmoist[i+3] *
			h[i+3] - theta[i+3] + theold[i+3];
	}
#else
	/* rest of layers */
	for (i = 1; i <= numeq; i++){
		thomb[i] = thomb[i] - capold[i] + diffmoist[i];
		thomf[i] = thomf[i] - capold[i] * hold[i] + diffmoist[i] *
			h[i] - theta[i] + theold[i];
	}

#endif
	/* Exception for lysimeter bottom */
	if (lbc == 0){
		thoma[numeq] = 0.0;
		thomc[numeq] = 0.0;
		thomb[numeq] = 1.0;
		thomf[numeq] = h[ngwl];
	}else if (lbc == 4){
		thoma[numeq] = 0.0;
		thomc[numeq] = 0.0;
		thomb[numeq] = 1.0;
		thomf[numeq] = h[numeq];
	}

}

/*C:h_by_tridiag
 *@ int h_by_tridiag()
 * 
 * Description: Solves the matrix by the simple and fast algorithm for
 * a tridiagonal matrix. If a zero pivot occurs it exits and returns 1
 * and the more general method for solving band-diagonal matrixes
 * should be used. 0 is returned on success
 *
 * Returns: 0 on sucess, 1 on failure*/
int
h_by_tridiag ()
{
	int i;
	double bet;

	nr_tri++;
	bet = thomb[0];
	if (bet == 0.0)
		return 1;


	greekc[0] = thomf[0] / bet;
	for (i = 1; i <= numeq; i++){
		greekb[i] = thomc[i - 1] / bet;
		if ((bet = thomb[i] - thoma[i] * greekb[i]) == 0.0)
			return 1;

		greekc[i] = (thomf[i] - thoma[i] * greekc[i - 1]) / bet;
	}

	h[numeq] = greekc[numeq];
	for (i = numeq - 1; i >= 0; i--){
		h[i] = greekc[i] - greekb[i + 1] * h[i + 1];
	}
	return 0;
}


/*C:savevars
 *@ void savevars()
 * 
 * Description: Saves spare copies of the h, theta and diffmoist arrays
 *
 * Returns: nothing*/ 
void 
savevars()
{
#ifdef HAVE_MEMCPY
	memcpy((double *)hold,(double *)h,layers * sizeof(double));
	memcpy((double *)theold,(double *)theta,layers * sizeof(double));
	memcpy((double *)capold,(double *)diffmoist,layers * sizeof(double));
#else
	int i;

	for (i=0;i<layers;i++){
		hold[i]=h[i];
		theold[i] = theta[i];
		capold[i] = diffmoist[i];
	}
#endif
}

/*C:h_by_banddiag
 *@ void h_by_banddiag()
 * 
 * Description: Solves the tridiagonal matrix as if it was a more
 * general band-diagonal matrix. This is only needed if a zero pivot
 * occurs in tridiag. This is seldom needed.
 *
 * Returns: nothing+*/
void
h_by_banddiag()
{
	int i;
	extern void New_mprove(double **a, double **alud, int n, int indx[], double b[], double x[]);

	nr_band++;
#ifdef HAVE_MEMCPY
	memcpy((double *)thomfhold,(double *)thomf,layers * sizeof(double));
#else
	for (i = 0; i < layers; i++){
		thomfhold[i] = thomf[i];
	}
#endif

	/* These are in band.c */
	fillmat (mat,thoma, thomb, thomc);
	bandec (mat, layers, a1, indx);
	banks (mat, layers, a1, indx, (&thomf[-1]));
	/*
	New_mprove(mat,a1,layers, indx, (&thomfhold[-1]),(&thomf[-1]));
	New_mprove(mat,a1,layers, indx, (&thomfhold[-1]),(&thomf[-1]));
	*/
	h[numeq] = thomf[numeq];
	for (i = numeq - 1; i >= 0; i--)
		h[i] = thomf[i];

	/* we need to keep the old values of thomf*/
#ifdef HAVE_MEMCPY
	memcpy((double *)thomf,(double *)thomfhold,layers * sizeof(double));
#else
	for (i = 0; i < layers; i++)
		thomf[i] = thomfhold[i];
#endif
}


/*C:h_satu
 *@ void h_satu()
 * 
 * Changes the profile from completely saturated to almost saturated
 *
 * Returns: nothing */
void
h_satu()
{
	int i;

	nr_sat++;
	for (i=0; i< layers; i++)
		qtop = fabs(qtop) > fabs(node[i].sp->ksat) ? -node[i].sp->ksat
			: qtop;
/*	theta[0] -= (qbot-qtop-qdrtot)*dt/dz[0];
	h[0]=node[0].sp->t2h (node[0].soiltype,theta[0], depth[0]);*/
	for (i=0; i < layers; i++){
		h[i] = depth[i];
	}
}

/*C:h_itt
 *@ void h_itt()
 * Calculates new head via LU decomposition and uses 2 xtra itterations
 * to get full machine precision in the solution. It is the slowest
 * of the three possible solutions.*/
void
h_itt()
{
	int i;
	double **tmpmat;
	double **storemat;
	int *indx;
	double dd;
	extern double **expand_mat(double **mat,int n,int m1,int m2);
	extern void mprove(double **a,double **alud, int n, int indx[], double b[], double x[]);
	extern void ludcmp(double **a, int n, int *indx, double *d);
	extern void lubksb(double **a, int n, int *indx, double b[]);

	nr_hitt++;
	indx = ivector(1,layers);
#ifdef HAVE_MEMCPY
	memcpy((double *)thomfhold,(double *)thomf,layers * sizeof(double));
#else
	for (i = 0; i < layers; i++)
		thomfhold[i] = thomf[i];
#endif

	fillmat (mat,thoma, thomb, thomc);
	tmpmat = expand_mat(mat,layers,1,1); 
	storemat = expand_mat(mat,layers,1,1); 
	ludcmp(tmpmat,layers,indx,&dd);
	lubksb(tmpmat,layers,indx,(&thomf[-1]));
	mprove(storemat,tmpmat,layers, indx, (&thomfhold[-1]),(&thomf[-1]));
	mprove(storemat,tmpmat,layers, indx, (&thomfhold[-1]),(&thomf[-1]));

	h[numeq] = thomf[numeq];
	for (i = numeq - 1; i >= 0; i--)
		h[i] = thomf[i];

	/* we need to keep the old values of thomf*/
#ifdef HAVE_MEMCPY
	memcpy((double *)thomf,(double *)thomfhold,layers * sizeof(double));
#else
	for (i = 0; i < layers; i++)
		thomf[i] = thomfhold[i];
#endif

	free_dmatrix(tmpmat,1,layers,1,layers);
	free_ivector(indx,1,layers);
	free_dmatrix(storemat,1,layers,1,layers);
}
