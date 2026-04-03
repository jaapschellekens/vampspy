#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "nrutil.h"


#ifdef NR_MEM_DEBUG
extern unsigned long nr_dm = 0;
extern unsigned long nr_m = 0;
extern unsigned long nr_im = 0;
#endif

#ifdef NR_LIST
extern int cldfrf;
extern int add_m(int type, void *vp, int fr, int lr, int fc, int lc, char *desc);
#endif

/*C:matrix
 *@ float **matrix(long nrl, long nrh, long ncl, long nch)
 *
 * Allocate a float matrix with subscript range @m[nrl..nrh][ncl..nch]@ */
float **matrix(long nrl, long nrh, long ncl, long nch)
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

#ifdef NR_MEM_DEBUG
	nr_m++;
#endif
	/* allocate pointers to rows */
	m=(float **) malloc((size_t)((nrow+NR_END)*sizeof(float*)));
	if (!m) l_error(1,"matrix","could not allocate %d row pointers\n",nrow);
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(float *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float)));
	if (!m[nrl]) l_error(1,"matrix","could not allocate %d-%d matrix\n",nrow,ncol);
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

#ifdef NR_LIST
	add_m(NR_FM, (void *)m, nrl, nrh, ncl, nch, "FLOAT MAT");
#endif
	/* return pointer to array of pointers to rows */
	return m;
}

/*C:dmatrix
 *@ double **matrix(long nrl, long nrh, long ncl, long nch)
 *
 * Allocate a double matrix with subscript range @m[nrl..nrh][ncl..nch]@ */
double **dmatrix(long nrl, long nrh, long ncl, long nch)
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

#ifdef NR_MEM_DEBUG
	nr_dm++;
#endif
	/* allocate pointers to rows */
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	if (!m) l_error(1,"dmatrix","could not allocate %d row pointers\n",nrow);
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	if (!m[nrl]) l_error(1,"dmatrix","could not allocate %d-%d matrix\n",nrow,ncol);
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

#ifdef NR_LIST
	add_m(NR_DM, (void *)m, nrl, nrh, ncl, nch, "DOUBLE MAT");
#endif

	/* return pointer to array of pointers to rows */
	return m;
}

/*C:imatrix
 *@ int **imatrix(long nrl, long nrh, long ncl, long nch)
 *
 * Allocate an int matrix with subscript range @m[nrl..nrh][ncl..nch]@ */
int **imatrix(long nrl, long nrh, long ncl, long nch)
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	int **m;

#ifdef NR_MEM_DEBUG
	nr_im++;
#endif
	/* allocate pointers to rows */
	m=(int **) malloc((size_t)((nrow+NR_END)*sizeof(int*)));
	if (!m) l_error(1,"imatrix","could not allocate %d row pointers\n",nrow);
	m += NR_END;
	m -= nrl;


	/* allocate rows and set pointers to them */
	m[nrl]=(int *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(int)));
	if (!m[nrl]) l_error(1,"imatrix","could not allocate %d-%d matrix\n",nrow,ncol);
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

#ifdef NR_LIST
	add_m(NR_IM, (void *)m, nrl, nrh, ncl, nch, "INT MAT");
#endif
	/* return pointer to array of pointers to rows */
	return m;
}

float **submatrix(float **a, long oldrl, long oldrh, long oldcl, long oldch,
	long newrl, long newcl)
/* point a submatrix [newrl..][newcl..] to a[oldrl..oldrh][oldcl..oldch] */
{
	long i,j,nrow=oldrh-oldrl+1,ncol=oldcl-newcl;
	float **m;

	/* allocate array of pointers to rows */
	m=(float **) malloc((size_t) ((nrow+NR_END)*sizeof(float*)));
	if (!m) l_error(1,"submatrix","could not allocate %d row pointers\n",nrow);
	m += NR_END;
	m -= newrl;

	/* set pointers to rows */
	for(i=oldrl,j=newrl;i<=oldrh;i++,j++) m[j]=a[i]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

float **convert_matrix(float *a, long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix m[nrl..nrh][ncl..nch] that points to the matrix
declared in the standard C manner as a[nrow][ncol], where nrow=nrh-nrl+1
and ncol=nch-ncl+1. The routine should be called with the address
&a[0][0] as the first argument. */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

	/* allocate pointers to rows */
	m=(float **) malloc((size_t) ((nrow+NR_END)*sizeof(float*)));
	if (!m) l_error(1,"convert_matrix","could not allocate %d row pointers\n",nrow);
	m += NR_END;
	m -= nrl;

	/* set pointers to rows */
	m[nrl]=a-ncl;
	for(i=1,j=nrl+1;i<nrow;i++,j++) m[j]=m[j-1]+ncol;
	/* return pointer to array of pointers to rows */
	return m;
}

double **convert_dmatrix(double *a, long nrl, long nrh, long ncl, long nch)
/* allocate a double matrix m[nrl..nrh][ncl..nch] that points to the matrix
declared in the standard C manner as a[nrow][ncol], where nrow=nrh-nrl+1
and ncol=nch-ncl+1. The routine should be called with the address
&a[0][0] as the first argument. */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((size_t) ((nrow+NR_END)*sizeof(double *)));
	if (!m) l_error(1,"convert_dmatrix","could not allocate %d row pointers\n",nrow);
	m += NR_END;
	m -= nrl;

	/* set pointers to rows */
	m[nrl]=a-ncl;
	for(i=1,j=nrl+1;i<nrow;i++,j++) m[j]=m[j-1]+ncol;
	/* return pointer to array of pointers to rows */
	return m;
}

/*C:free_matrix
 *@ void free_matrix(float **m, long nrl, long nrh, long ncl, long nch)
 *
 * Free a float matrix allocated by @matrix()@ */
void free_matrix(float **m, long nrl, long nrh, long ncl, long nch)
{
#ifdef NR_MEM_DEBUG
	nr_m--;
#endif
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)m);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
	m = NULL;
}

/*C:free_dmatrix
 *@ void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch)
 *
 * Free a double matrix allocated by @dmatrix()@ */
void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch)
{
#ifdef NR_MEM_DEBUG
	nr_dm--;
#endif
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)m);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
	m = NULL;
}

/*C:free_imatrix
 *@ void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch)
 *
 * Free a int matrix allocated by @imatrix()@ */
void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch)
{
#ifdef NR_MEM_DEBUG
	nr_im--;
#endif
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)m);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
	m = NULL;
}

void free_submatrix(float **b, long nrl, long nrh, long ncl, long nch)
/* free a submatrix allocated by submatrix() */
{
	free((FREE_ARG) (b+nrl-NR_END));
	b = NULL;
}

void free_convert_matrix(float **b, long nrl, long nrh, long ncl, long nch)
/* free a matrix allocated by convert_matrix() */
{
	free((FREE_ARG) (b+nrl-NR_END));
	b = NULL;
}
