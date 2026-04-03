/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/band.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: band.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

/*
 * This stuff has been derived from the Numerical recipies pascal
 * version.
 */

#include <math.h>
#include "swatsoil.h"
#include "vamps.h"
#include "nrutil.h"
#include "marquard.h"


static double dumm;
#define SWAP(a,b) {dumm=(a);(a)=(b);(b)=dumm;}
#define TINY 1.0E-20


/*C:fillmat
 *@ void fillmat(double **mat,double a[], double b[], double c[])
 *  Description: Fills the matrix mat in the compact 45oC rotated form
 *  Returns: nothing
 */ 
void fillmat(double **mat,double a[],double b[],double c[])
{
	int i;
  
	mat[1][2] = b[0];
	mat[1][3] = c[0];
	mat[1][1] = 0.0;
	for (i=1;i<layers-1;i++){
		mat[i+1][1]=a[i];
		mat[i+1][2]=b[i];
		mat[i+1][3]=c[i];
	}
	mat[layers][1] = a[layers-1];
	mat[layers][2] = b[layers-1];
	mat[layers][3] = 0.0;
}

void
prmat(char *fname, double **mat,int sr, int nr,int sc, int nc)
{
	int i,j;
	FILE *of;

	of = fopen(fname,"w");

	for (i=sr; i<=nr; i++){
		for(j=sc;j<=nc; j++){
			fprintf(of,"%g ",mat[i][j]);
		}
		fprintf(of,"\n");
	}
	(void)fclose(of);
}

double **expand_mat(double **mat,int n,int m1,int m2)
{
	int i,j,k,tmp;
	double mx = 0.0;
	double **ret;
	
	ret =  dmatrix(1,n,1,n); nr_descr("Return matrix in expand_mat",(void *)ret);

	for (i=1;i<=n;i++)
		for (j=1;j<=n;j++)
			ret[i][j] = 0.0;

	/* This is from Slang, can be rewritten */
	for (i = 1; i <= n; i++){
		k = i - m1 - 1;
		if (n - k < m1 + m2 + 1)
			tmp = n - k + 1;
		else
			tmp = m2 + m1 + 2;

		for (j = 1; j <= n; j++){
			if ( j < tmp)
				mx = mat[i][j];
			else
				mx = 0.0;

			k = i + j - 2;
			if ((k <= n) && (k > 0)){
				ret[i][k] = mx;
			}
		}
	}

	return ret;
}


void
bandec (double **a, int n, double **a1, int indx[])
{
	int  i, j, k, l;
	double  dum;

	l = 1;

	for (i = 1; i <= 1; i++){
		for (j = 1 + 2 - i; j <= 3; j++)
			a[i][j - l] = a[i][j];
		l--;
		for (j = 3 - l; j <= 3; j++)
			a[i][j] = 0.0;
	}
	l = 1;

	for (k = 1; k <= n; k++){
		dum = a[k][1];
		i = k;
		if (l < n)
			l++;
		for (j = k + 1; j <= l; j++){
			if (fabs (a[j][1]) > fabs (dum)){
				dum = a[j][1];
				i = j;
			}
		}

		indx[k] = i;
		if (dum == 0.0){
			a[k][1] = TINY;
			fprintf(stderr,"\nzero pivot!!\n\n");
		}

		if (i != k){
			for (j = 1; j <= 3; j++)
				SWAP (a[k][j], a[i][j]);
		}

		for (i = k + 1; i <= l; i++){
			dum = a[i][1] / a[k][1];
			a1[k][i - k] = dum;
			for (j = 2; j <= 3; j++)
				a[i][j - 1] = a[i][j] - dum * a[k][j];
			a[i][3] = 0.0;
		}
	}
}

void
banks(double **a,int n, double **a1, int indx[], double b[])
{
	int i,k,l;
	double dum;

	l=1;

	for (k=1;k<=n; k++){
		i = indx[k];
		if (i !=k)
			SWAP(b[k],b[i]);
		if (l<n) l++;
		for (i=k+1;i<=l;i++)
			b[i] -= a1[k][i-k]*b[k];
	}
	l=1;

	for (i=n;i>=1;i--){
		dum=b[i];
		for (k=2;k<=l;k++)
			dum -= a[i][k]*b[k+i-1];
		b[i]=dum/a[i][1];
		if (l<3) l++;
	}
}

/* iterative improvement */      

void lubksb(double **a, int n, int *indx, double b[])
{
	int i,ii=0,ip,j;
	double sum;

	for (i=1;i<=n;i++){
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii)
			for (j=ii; j<=i-1; j++)
				sum -= a[i][j] * b[j];
		else if(sum) ii = i;
		b[i] = sum;
	}
	for (i=n;i>=1;i--){
		sum=b[i];
		for (j= i+1; j<=n;j++)
			sum -= a[i][j] * b[j];
		b[i]=sum/a[i][i];
	}
}


void
ludcmp(double **a, int n, int *indx, double *d)
{
	int i,imax,j,k;
	double big,dum,sum,temp;
	double *vv;

	vv = dvector(1,n);

	*d = 1.0;
	for (i=1;i<=n;i++){
		big = 0.0;
		for(j=1;j<=n;j++)
			if ((temp=fabs(a[i][j])) > big) big = temp;
		if (big == 0.0){
			fprintf(stderr,"Shit, Singular matrix, dumping matrix to dump.mat");
			prmat("dump.mat", a,1,n,1,n);
			exit(1);
		}
		vv[i] = 1.0/big;
	}
	for(j=1;j<=n;j++){
		for (i=1;i<j;i++){
			sum = a[i][j];
			for (k=1; k<i;k++)
				sum -= a[i][k] * a[k][j];
			a[i][j] = sum;
		}
		big = 0.0;
		for(i=j;i<=n;i++){
			sum=a[i][j];
			for(k=1;k<j;k++)
				sum -= a[i][k] * a[k][j];
			a[i][j] = sum;
			if ((dum = vv[i] * fabs(sum)) >= big){
				big = dum;
				imax = i;
			}
		}
		if (j != imax){
			for (k=1;k<=n;k++){
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			*d = -(*d);
			vv[imax]=vv[j];
		}
		indx[j] = imax;
		if (a[j][j] == 0.0){
			fprintf(stderr,"zero pivot\n");
			a[j][j] = TINY;
		}

		if (j != n){
			dum = 1.0/a[j][j];
			for (i=j+1;i<=n;i++)
				a[i][j] *= dum;
		}
	}

	free_dvector(vv,1,n);
}

void
mprove(double **a, double **alud, int n, int indx[], double b[], double x[])
{
	int i,j;
	double sdp;
	double *r;

	r = dvector(1,n);

	for (i = 1; i <= n; i++){
		sdp = -b[i];
		for (j=1; j <=n; j++)
			sdp += a[i][j] * x[j];
		r[i] = sdp;
	}
	lubksb(alud,n,indx,r);
	for (i=1; i <=n; i++)
		x[i] -= r[i];

	free_dvector(r,1,n);
}

void
New_mprove(double **a, double **alud, int n, int indx[], double b[], double x[])
{
	int i,j;
	double sdp;
	double *r;

	r = (double *) ts_memory(NULL,(1 + n) * sizeof (double),progname);

	for (i = 1; i <= n; i++){
		sdp = -b[i];
		for (j=1; j <=3; j++){
			sdp += a[i][j] * x[j];
		}
		r[i] = sdp;
	}
	banks(a, n, alud,indx,r);
	for (i=1; i <=n; i++)
		x[i] -= r[i];

	free(r);
}
