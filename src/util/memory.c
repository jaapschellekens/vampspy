/*C:memory
 *@ void *memory(void *ptr, size_t size, char *progname)
 *		Allocates size bytes of memory if ptr == NULL, else
 *		reallocates size bytes from ptr (ANSI realloc()). On
 *		memory error, print diagnostic with progname and exit
 *		with status 1.
 *	Ret:	pointer to allocated memory, deallocate with free(3).
 */
#include <sys/types.h>
#include <stdio.h>
#include "vamps.h"

void *
memory(void *ptr,size_t size,char *progname)
{
	void	*rp;

	if(ptr)
		rp = (void *)realloc(ptr, size);
	else
		rp = (void *)malloc(size);
	if(rp)
		return(rp);

	(void)fprintf(stderr, "%s: Memory allocation failed\n", progname);
	exit(1);
	/* NOTREACHED */
}
