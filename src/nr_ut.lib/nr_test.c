#include "nrutil.h"


int
main()
{
	double **a,**cc;
	float  **b;
	double **h;
	double *ex;
	double *mean;
	int	**c;
	int nr = 0, nc= 1;

	a = dmatrix(0,100,0,6);
	a[1][0] = 100.0;
	a[1][3] = 100.0;
	nr_descr("Testing 0--100, 0--2 double matrix",(void *)a);
	cc = todxy((void *)a,0,3);
	b = matrix(10,19,2,3);
	b[10][2] = 10;
	b[11][2] = 10;
	b[13][2] = 10;
	b[12][2] = 10;
	b[19][2] = 10;
	mean = nr_mean((void *)b);
	nr_descr("Mean of columns from b",(void *)mean);
	nr_genw("test5",(void *)mean);
	nr_genw("test6",(void *)cc);
	nr_descr("Testing 10--20, 2--3 float matrix",(void *)b);
	c = imatrix(0,100,0,2);
	nr_descr("Testing 0--100, 0--2 integer matrix",(void *)c);
	ex = todvec((void *)b,2);

	nr_descr("Ex from  b",(void *)ex);
	/* write to a file ...*/
	nr_genw("test0",(void *)a);
	nr_genw("test1",(void *)b);
	nr_genw("test2",(void *)c);
	nr_genw("test3",(void *)ex);

	/* Free them ...*/
	nr_free_substr("MAT");
	nr_free_all();

	/* Now read test1 in a double matrix forcing only on column */
	h = nr_dmread("test1",&nr,&nc);
	/* write it out */
	nr_genw("test4",(void *)h);
	/* free it */
	nr_free(1,(void *)h);
}
