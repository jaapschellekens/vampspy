/* $Header: /home/schj/src/vamps_0.99g/src/ts.lib/RCS/ts_mem.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ */

/*  $RCSfile: ts_mem.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts.h"

#ifndef NOMEM_EXIT
#define NOMEM_EXIT 8
#endif

static char RCSid[] =
"$Id: ts_mem.c,v 1.7 1999/01/06 12:13:01 schj Alpha $";

/*C:ts_memory
 *@void *ts_memory(void *ptr, size_t size, char *progname)
 *
 * Allocates @size@ bytes of memory if @ptr == NULL@, else
 * reallocates @size@ bytes from @ptr@ (ANSI @realloc()@). On
 * memory error, print diagnostic with @progname@ and exit
 * with status @NOMEM_EXIT@.
 * Ret: pointer to allocated memory, deallocate with @free(3)@.
 */
void *
ts_memory(void *ptr,size_t size,const char *progname)
{
	void	*rp;

	if(ptr)
		rp = (void *)realloc(ptr, size);
	else
		rp = (void *)malloc(size);
	if(rp)
		return(rp);

	(void)fprintf(stderr, "%s: Memory allocation failed\n(%s)\n", progname,RCSid);
	exit(NOMEM_EXIT);
	/* NOTREACHED */
}
