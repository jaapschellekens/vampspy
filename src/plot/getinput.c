#include <sys/types.h>
#include <ctype.h>
#include "graph.h"

/* memory increment */
#define	M_INC	1024

extern double	strtod();
extern void	*malloc(), *realloc();

/*
 * reads input file [x] y [["]label["]]
 *	return: total number coordinates in data array
 */
getinput2(fp, crp, np, autoabs, start, step)
FILE	*fp;
Coord2	**crp;
int	np, autoabs;
double	start, step;
{
	char	*cp, *rp, buf[BUFSIZ];
	static int	len = 0;
	static Coord2	*cr = (Coord2 *)NULL;

	while(cp = fgets(buf, BUFSIZ, fp)) {
		while(isspace(*cp))	/* skip whitespace */
			cp++;
		if(*cp == '#')		/* ignore comment */
			continue;

	/* reallocate memory if needed */
		while(np >= len) {
			len += M_INC;
			cr = (Coord2 *)memory((void *)cr, len * sizeof(Coord2),"plot");
		}

	/* get numerical fields, cp now points to first field */
		if(autoabs) {
			cr[np].x = start;
			start += step;
		}
		else {
			if((cr[np].x = strtod(cp, &rp)) == 0.0 && cp == rp)
				return(-1);	/* input error */
			cp = rp;
		}
		if((cr[np].y = strtod(cp, &rp)) == 0.0 && cp == rp)
			return(-1);	/* input error */

	/* get label, if any, from rp */
		while(isspace(*rp))
			rp++;
		if(*rp == '"' || *rp == '\n')
			rp++;
		if(*rp) {
			cp = cr[np].s = (char *)memory((void *)NULL, strlen(rp) + 1,"plot");
			while(*rp && *rp != '\n' && *rp != '"')
				*cp++ = *rp++;
			*cp = '\0';	/* terminate with NUL */
		}
		else
			cr[np].s = (char *)NULL;
		np++;
	}

	*crp = cr;

	return(ferror(fp) ? -1 : np);
}
