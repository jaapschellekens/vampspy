#include <stdio.h>
#include "plot.h"

#ifdef __STDC__
#include <sys/cdefs.h>
#else
#define __P(a)	()
#define	const
#endif

#define X_AXIS	1
#define Y_AXIS	2

#define GRIDNONE	0
#define GRIDFULL	1
#define GRIDALLTICKS	2
#define GRIDLBTICKS	3
#define GRIDORIGAXES	4

/*
 * coordinate data type
 */
typedef struct {
	double	x, y;	/* x,y value */
	char	*s;	/* label string */
} Coord2;

/*
 * coordinate conversion macros
 */
#define xcvt(m,x,y)	(int)(m[0] * (x) + m[2] * (y) + m[4] + 0.5)
#define ycvt(m,x,y)	(int)(m[1] * (x) + m[3] * (y) + m[5] + 0.5)

extern int	getinput2 __P((FILE *, Coord2 **, int, int, double, double));
extern void	getrange __P((Coord2 *p, int np,
				double *x_min, double *y_min,
				double *x_max, double *y_max));

extern double	scale1 __P((double vmin, double vmax, int ntics));
extern void	getmatrix __P((double mtx[]));
extern void	setmatrix __P((double x0, double y0, double x1, double y1,
				int transpose));

extern void	getviewport __P((int *x0, int *y0, int *x1, int *x2));
extern void	setviewport __P((double x0, double y0, double x1, double y1));

extern int	draw_graph __P((Coord2 *p, int np,
				int linemode, int symbol, int symsize));

extern int	draw_frame __P((int grid, int ticklen,
				double x_min, double x_max, double x_spc,
				double y_min, double y_max, double y_spc,
				char *x_title, char *y_title, char *p_title,
				int log_axis, int no_labels));
