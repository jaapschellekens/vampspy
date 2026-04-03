/*- 
 * void *memory(void *ptr, size_t size, char *progname)
 *		Allocates size bytes of memory if ptr == NULL, else
 *		reallocates size bytes from ptr (ANSI realloc()). On
 *		memory error, print diagnostic with progname and exit
 *		with status 1.
 *	Ret:	pointer to allocated memory, deallocate with free(3).
 *
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "marquard.h"

void *
m_memory(void *ptr,size_t size,const char *progname)
{
	void	*rp;

	if(ptr)
		rp = (void *)realloc(ptr, size);
	else
		rp = (void *)malloc(size);
	if(rp)
		return(rp);

	(void)fprintf(stderr, "%s: Memory allocation failed\n", progname);
	exit (1);
	/* NOTREACHED */
}


double **m_matr (int rows,int  cols)
{
    /* allocates a double matrix */

    register int i;
    register double **m;

    if ( rows < 1  ||  cols < 1 )
        return NULL;
    /* allocate pointers to rows */
    if ( (m = (double **) malloc ((rows)* sizeof(double *) )) == NULL ){
	fprintf (stderr,"Memory running low during fit: rows = %d\n", rows);
	exit (1);
    }
    /* allocate rows and set pointers to them */
    for ( i=0 ; i<rows ; i++ )
	if ( (m[i] = (double *) malloc ((cols)* sizeof(double))) == NULL ){
	    fprintf (stderr,"Memory running low during fit: cols = %d\n",cols);
	    exit (1);
	}
    return m;
}

void m_free_matr (double **m,int  rows)
{
    register int i;
  
    for ( i=0 ; i<rows ; i++ ){
      free ( m[i] );
    }
    free (m);
}
