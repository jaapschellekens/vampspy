#include "graph.h"

/*
 * draw_graph draws an x,y graph from the data
 */
int
draw_graph(p, np, linemode, symbol, symsize)
Coord2	*p;
int	np, linemode, symbol, symsize;
{
	int	i;			/* index in data array */
	int	xv, yv;			/* point in plot space */
	int	need_move;		/* set if we need a move */
	int	llx, lly, urx, ury;	/* the graph viewport limits */
	int	ds1, ds2, ds3;		/* scaled symbol sizes */
	double	ctm[6];			/* coor --> plot matrix */
	extern char	*linemodes[];	/* in graph.c */

	getmatrix(ctm);
	getviewport(&llx, &lly, &urx, &ury);
	ds1 = (symsize + 1) / 2;
	ds2 = (int)((double)ds1 * 0.5 + 0.5);
	ds3 = (int)((double)ds1 * 0.867 + 0.5);

	if(linemode) {	/* do linework */
		(void)linemod(linemodes[linemode]);
		for(i = 0; i < np; i++) {
			if(!i || p[i].x < p[i-1].x)	/* new dataset */
				need_move = 1;

			xv = xcvt(ctm, p[i].x, p[i].y);
			yv = ycvt(ctm, p[i].x, p[i].y);

			/* should clip */
			if(xv < llx || xv > urx || yv < lly || yv > ury) {
				need_move = 1;
				continue;
			}
			if(need_move) {
				need_move = 0;
				(void)move(xv, yv);
			}
			else
				(void)cont(xv, yv);
		}
	}
	if(symbol && linemode != 1)
		(void)linemod("solid");
	for(i = 0; i < np; i++) {	/* do symbols/labels */
		xv = xcvt(ctm, p[i].x, p[i].y);
		yv = ycvt(ctm, p[i].x, p[i].y);

		if(xv < llx || xv > urx || yv < lly || yv > ury)
			continue;

		switch(symbol) {
		case 0:
			break;
		case 1:		/* plus */
			(void)line(xv, yv - ds1, xv, yv + ds1);
			(void)line(xv - ds1, yv, xv + ds1, yv);
			break;
		case 2:		/* cross */
			(void)line(xv - ds1, yv + ds1, xv + ds1, yv - ds1);
			(void)line(xv - ds1, yv - ds1, xv + ds1, yv + ds1);
			break;
		case 3:		/* star */
			(void)line(xv - ds1, yv, xv + ds1, yv);
			(void)line(xv - ds2, yv + ds3, xv + ds2, yv - ds3);
			(void)line(xv - ds2, yv - ds3, xv + ds2, yv + ds3);
			break;
		case 4:		/* box */
			(void)move(xv + ds1, yv + ds1);
			(void)cont(xv - ds1, yv + ds1);
			(void)cont(xv - ds1, yv - ds1);
			(void)cont(xv + ds1, yv - ds1);
			(void)cont(xv + ds1, yv + ds1);
			break;
		case 5:		/* diamond */
			(void)move(xv + ds1, yv);
			(void)cont(xv, yv + ds1);
			(void)cont(xv - ds1, yv);
			(void)cont(xv, yv - ds1);
			(void)cont(xv + ds1, yv);
			break;
		case 6:		/* circle */
			circle(xv, yv, ds1);
			if(p[i].s)
				(void)move(xv + ds1, yv);
			break;
		}
		if(p[i].s) {
			if(!symbol)
				(void)move(xv, yv);
			alabel('l', 'c', p[i].s);
		}
	}
	return(0);
}
