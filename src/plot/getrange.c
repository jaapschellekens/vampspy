#include "graph.h"

void
getrange(cr, np, x_min, y_min, x_max, y_max)
Coord2	*cr;
int	np;
double	*x_min, *y_min, *x_max, *y_max;
{
	int	i;
	double	xmin, ymin, xmax, ymax;

	if(np) {
		xmin = xmax = cr[0].x;
		ymin = ymax = cr[0].y;

		for(i = 1; i < np; i++) {
			if(cr[i].x < xmin)
				xmin = cr[i].x;
			else if(cr[i].x > xmax)
				xmax = cr[i].x;
			if(cr[i].y < ymin)
				ymin = cr[i].y;
			else if(cr[i].y > ymax)
				ymax = cr[i].y;
		}
	}
	else
		xmin = ymin = xmax = ymax = 0.0;

	if(x_min)
		*x_min = xmin;
	if(y_min)
		*y_min = ymin;
	if(x_max)
		*x_max = xmax;
	if(y_max)
		*y_max = ymax;

	return;
}
