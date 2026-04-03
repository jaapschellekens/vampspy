/* ts.h */

#ifndef TS_H_
#define TS_H_

#include <stdio.h>
#include <time.h>
#include <malloc.h> /* to use free() */
#include "version.h"

typedef struct tm TM;

typedef struct {
	double	x;	/* x coordinate */
	double	y;	/* y coordinate */
} XY;

/* default parameters
*/
#define DIG_TTY		"/dev/ttyb"
#define DIG_DELTA	4.0
#define S5002M		2.0
#define LSF4M		1.0
#define TIME_OFMT	"%d/%m/%Y %H:%M:%S "
#define JDATE_OFMT	"%.8f"
#define VALUE_OFMT	" %.8f"

/* ts_getinput() return values
*/
#define INP_OTHER	0	/* none of the below */
#define INP_COMMENT	1	/* # */

#ifdef __STDC__ 
#include <sys/cdefs.h>
#else
#define __P(a)()
#endif

/* common functions
*/
extern char	*ts_command __P ((int argc, char *argv[]));
extern char	*ts_getdefault __P ((char *name));
extern void	ts_jday __P ((TM *tm, double *jul));
extern void	ts_jdate __P ((TM *tm, double jul));
extern int	ts_time __P ((char *str, char **eptr, TM *tm));
extern int	ts_getinput __P ((FILE *fp, char *buf));
extern int	ts_opendefaults __P ((void));
extern XY	*ts_getxy __P ((char *buf, XY *xy));
extern void	ts_closedefaults __P ((void));
extern void	*ts_memory __P ((void *ptr, size_t size, char *progname));
extern XY 	*ts_readf __P ((int *points,FILE *datfile, char *prgname, int verbose));
extern double	*ts_slopes __P ((XY xy[], int np));
extern int	ts_meval __P ((XY val[], XY tab[], double mtab[], int nt, int nv));

#endif /* TS_H_ */
