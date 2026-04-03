/* $Header: /home/schj/src/vamps_0.99g/src/ts.lib/RCS/ts_readf.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ */

/*  $RCSfile: ts_readf.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#include "ts.h"
#include <stdlib.h>
#include <stdio.h>

static char RCSid[] =
"$Id: ts_readf.c,v 1.7 1999/01/06 12:13:01 schj Alpha $";

/*C:ts_readf
 *@XY *ts_readf(int *points, FILE *datfile, char *prgname, int xcol,
 *@		int ycol, int verbose)
 *
 * @ts_readf@ reads an ts datafile and returns a pointer to an XY structure.
 * This may be passed to @free()@ to free the allocated memory.
 * To support multicolumn files xcol and xcol can be specified.
 * @points@ will contain the number of points read. If verbose > 0 
 * comments in the datafile will be send to stderr. If verbose > 1
 * some file statistics will be printed as well. If verbose > 2
 * even more will be printed.
 */

XY 
*ts_readf(int *points,FILE *datfile,char *prgname,int xcol, int ycol,
		int verbose)
{
	XY	*xy=(XY *)NULL;
	XY	*ttxy;
	int	ret;
	int	done=0;
	int	step=1024;
	char	s[1024];
	int	i=0;
	int	j=0;

	while ((ret = ts_getinput (datfile, s)) != EOF){
		j++; /* Total nr of lines, including comments */
		if (ret == INP_OTHER){
			i++;
			if (i == 1){
				xy = (XY *) ts_memory ((void *) xy, step * (sizeof (XY)),prgname);
				done = step;
			}else if (i > done){
				xy = (XY *) ts_memory ((void *) xy, (step + done) * sizeof (XY),prgname);
				done = done + step;
			}
			if ((ttxy = ts_getxy (s, (XY *) NULL,xcol,ycol))==(XY *)NULL){
				(void)fprintf(stderr,"%s: Error in input, line %d\n",prgname,j);
				free(xy);
				exit(1);
			}else{
				xy[i - 1].y =  ttxy->y;
				xy[i - 1].x =  ttxy->x;
			}
		}else{
			if (verbose>1) /* Copy comments to screen */{
				(void)fprintf(stderr,"comment: %s\n", s);
			}
		}
	}

	*points=i; /* actual data points */
	if (verbose)
		(void)fprintf(stderr,"%s: read %d points\n",RCSid,i);

	/* realloc xy... */
	xy = (XY *) ts_memory ((void *) xy, i * (sizeof (XY)),prgname);

	return xy;
}
