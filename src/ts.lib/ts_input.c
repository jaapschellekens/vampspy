/* $Header: /home/schj/src/vamps_0.99g/src/ts.lib/RCS/ts_input.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ */

/*  $RCSfile: ts_input.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static char RCSid[] =
"$Id: ts_input.c,v 1.7 1999/01/06 12:13:01 schj Alpha $";

/*C:ts_getinput
 *@int ts_getinput(FILE *fp, char buf[])
 *
 * Reads line from @fp@ into @buf@ which is expected at least
 * @BUFSIZ@ large. A possible terminating newline is replaced
 * by a @NUL@ character.
 * Ret: Retval depends on first character in input line:
 *@	INP_COMMENT	'#' comment line
 *@	INP_OTHER	any other (data) line
 *@	EOF		on end of file;
 */
#include <stdlib.h>
#include "ts.h"

int
ts_getinput(FILE *fp,char *buf)
{
	char	*cp;

	if(fgets(buf, BUFSIZ, fp) == (char *)NULL)
		return(EOF);

	for(cp = buf; *cp; cp++)
		if(*cp == '\n')
			*cp = '\0';

	if(buf[0] == '#')
		return(INP_COMMENT);

	return(INP_OTHER);
}

/*C:ts_getxy
 *@ XY *ts_getxy(char buf[], XY *xy,int xcol, int ycol)
 *
 * Scans the NUL terminated string @buf@ for @xy@ data values.
 * If @*xy != NULL@, the result is stored at the location it
 * points to.
 * Ret: Pointer to type XY holding the result or NULL on error.
 * Note: Retval points to static memory overwritten at each call.
 * Current implementation does not check errno.
 */
extern double strtod(const char *nptr, char **endptr);
XY *
ts_getxy(char *buf,XY *xy,int xcol,int ycol)
{
	char	*ept = NULL;
	static XY	res;
	int col = 0;
	double dm;

	ept = buf;
	do {
		buf = ept;
		dm = strtod (buf, &ept);
		if (col == xcol)
			res.x = dm;
		if (col == ycol)
			res.y = dm;
		col++;
	}
	while (buf != ept);


	if(xy) {
		xy->x = res.x;
		xy->y = res.y;
	}

	return(&res);
}
