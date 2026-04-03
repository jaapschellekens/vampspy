#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "nrutil.h"


#ifdef NR_MEM_DEBUG
extern  unsigned long nr_dv = 0;
extern  unsigned long nr_v = 0;
extern  unsigned long nr_iv = 0;
#endif

#ifdef NR_LIST
extern int cldfrf;
extern int add_m(int type, void *vp, int fr, int lr, int fc, int lc, char *desc);
#endif

/*C:vector
 *@float *vector(long nl, long nh)
 *
 * Allocate a float vector with subscript range @v[nl..nh]@.*/
float *vector(long nl, long nh)
{
	float *v;

#ifdef NR_MEM_DEBUG
	nr_v++;
#endif
	v=(float *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(float)));
	if (!v) l_error(1,"vector","could not allocate %d floats\n",nh-nl);

	v = v - nl + NR_END;
#ifdef NR_LIST
	add_m(NR_FV, (void *)v, nl, nh, 0, 0, "FLOAT VECT");
#endif
	return v;
}

/*C:ivector
 *@int *ivector(long nl, long nh)
 *
 * Allocate an int vector with subscript range @v[nl..nh]@ */
int *ivector(long nl, long nh)
{
	int *v;

#ifdef NR_MEM_DEBUG
	nr_iv++;
#endif
	v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
	if (!v) l_error(1,"ivector","could not allocate %d int\n",nh-nl);

	v = v - nl + NR_END;
#ifdef NR_LIST
	add_m(NR_IV, (void *)v, nl, nh, 0, 0, "INT VECT");
#endif
	return v;
}

/*C:cvector
 *@unsigned char *cvector(long nl, long nh)
 *
 * Allocate an unsigned char vector with subscript range @v[nl..nh]@ */
unsigned char *cvector(long nl, long nh)
{
	unsigned char *v;

	v=(unsigned char *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(unsigned char)));
	if (!v) l_error(1,"cvector","could not allocate %d chars\n",nh-nl);

	v = v - nl + NR_END;
#ifdef NR_LIST
	add_m(NR_CV, (void *)v, nl, nh, 0, 0, "CHAR VECT");
#endif
	return v;
}

/*C:lvector
 *@unsigned long *lvector(long nl, long nh)
 *
 * Allocate an unsigned long vector with subscript range @v[nl..nh]@ */
unsigned long *lvector(long nl, long nh)
{
	unsigned long *v;

	v=(unsigned long *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(long)));
	if (!v) l_error(1,"lvector","could not allocate %d longs\n",nh-nl);

	v = v - nl + NR_END;
#ifdef NR_LIST
	add_m(NR_LV, (void *)v, nl, nh, 0, 0, "LONG VECT");
#endif
	return v;
}

/*C:dvector
 *@double *dvector(long nl, long nh)
 *
 * Allocate a double vector with subscript range @v[nl..nh]@ */
double *dvector(long nl, long nh)
{
	double *v;

#ifdef NR_MEM_DEBUG
	nr_dv++;
#endif
	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	if (!v) l_error(1,"dvector","could not allocate %d doubles\n",nh-nl);

	v = v -nl + NR_END;
#ifdef NR_LIST
	add_m(NR_DV, (void *)v, nl, nh, 0, 0, "DOUBLE VECT");
#endif
	return v;
}

/*C:free_vector
 *@ void free_vector(float *v, long nl, long nh)
 *
 * Free a float vector allocated with @vector()@ */
void free_vector(float *v, long nl, long nh)
{
#ifdef NR_MEM_DEBUG
	nr_v--;
#endif
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)v);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (v+nl-NR_END));
	v = NULL;
}

/*C:free_ivector
 *@ void free_ivector(float *v, long nl, long nh)
 *
 * Free an int vector allocated with @ivector()@ */
void free_ivector(int *v, long nl, long nh)
{
#ifdef NR_MEM_DEBUG
	nr_iv--;
#endif
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)v);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (v+nl-NR_END));
	v = NULL;
}

/*C:free_cvector
 *@ void free_cvector(float *v, long nl, long nh)
 *
 * Free an unsigned char vector allocated with @cvector()@ */
void free_cvector(unsigned char *v, long nl, long nh)
{
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)v);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (v+nl-NR_END));
	v = NULL;
}

/*C:free_lvector
 *@ void free_lvector(float *v, long nl, long nh)
 *
 * Free an unsigned long vector allocated with @lvector()@ */
void free_lvector(unsigned long *v, long nl, long nh)
{
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)v);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (v+nl-NR_END));
	v = NULL;
}

/*C:free_dvector
 *@ void free_dvector(float *v, long nl, long nh)
 *
 * Free a double vector allocated with @dvector()@ */
void free_dvector(double *v, long nl, long nh)
{
#ifdef NR_MEM_DEBUG
	nr_dv--;
#endif
#ifdef NR_LIST
	if (!cldfrf)
		nr_free(0,(void *)v);
	cldfrf = 0;
#endif	
	free((FREE_ARG) (v+nl-NR_END));
	v = NULL;
}
