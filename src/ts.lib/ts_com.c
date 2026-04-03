/* $Header: /home/schj/src/vamps_0.99g/src/ts.lib/RCS/ts_com.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ */

/*  $RCSfile: ts_com.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#include <stdio.h>
#include <string.h>
#include "ts.h"

#define META	"# Command:"

static char RCSid[] =
"$Id: ts_com.c,v 1.7 1999/01/06 12:13:01 schj Alpha $";

/*C:ts_command
 *@char *ts_command(int argc, char *argv[])
 *
 * Builds a NULL terminated string of maximum BUFSIZ bytes
 * long holding "@# Command: argv[0] ... argv[argc-1]@".
 * Ret:	Pointer to the metadata string.
 * Note: Retval points to static memory overwritten at each call.
 */
char *
ts_command(int argc,char *argv[])
{
	int	len;
	char	*src, *dst, *sp;
	static char	buf[BUFSIZ];

	len = BUFSIZ - 5;
	dst = buf;

	for(src = META; *src; len--)
		*dst++ = *src++;

	while(len > 0 && argc--) {
		*dst++ = ' ';
		src = *argv++;
		if((sp = strpbrk(src, " \t")))
			len -= strlen(src) - 3;
		else
			len -= strlen(src) - 1;
		if(len <= 0)
			src = "...";
		else if(sp)
			*dst++ = '"';
		while(*src)
			*dst++ = *src++;
		if(sp)
			*dst++ = '"';
	}
	*dst = '\0';

	return(buf);
}
