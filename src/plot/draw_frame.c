#include <math.h>
#include "ts.h"
#include "graph.h"

/* 0.01 and (0.05 - 0.01) * plot extents */
#define L_OFF	40
#define T_OFF	165

#ifdef TS_HACK
TM tm;
char *tfmt="%x";
int ts_x=0;
#endif

/*
 * draw_frame draws the box, grid, etc; sets the ctm
 */
int
draw_frame(grid, ticklen,
	    x_min, x_max, x_spc, y_min, y_max, y_spc,
	    x_title, y_title, p_title,
	    log_axis, nolabels)
int	grid, ticklen;
double	x_min, x_max, x_spc, y_min, y_max, y_spc;
char	*x_title, *y_title, *p_title;
int	log_axis, nolabels;
{
	int	i, i0, i1, j;		/* tick mark indices */
	char	lbl[512];		/* for tick mark labels */
	double	ctm[6];			/* coor-->plot matrix */
	double	xval, yval;		/* tick mark values */
	int	xv, yv;			/* plot space coors */
	int	xor_y, yor_x;		/* y,x coors of origaxes */
	int	xl_off, yl_off;		/* label offsets from axes */
	int	xt_len, yt_len;		/* x,y tickmark length */
	int	llx, lly, urx, ury;	/* viewport */

	fspace(x_min, y_min, x_max, y_max);

/* determine axes positions */
	if(grid == GRIDORIGAXES) {
		yval = y_max * y_min > 0.0 ?
			(y_max < y_min ? y_max : y_min) : 0.0;
		xval = x_max * x_min > 0.0 ?
			(x_max < x_min ? x_max : x_min) : 0.0;
		xor_y = ycvt(ctm, xval, yval);
		yor_x = xcvt(ctm, xval, yval);
	}
	else {
		xor_y = lly;
		yor_x = llx;
	}
	if(ury - xor_y < xor_y - lly)	/* x axis in upper half */
		xl_off = L_OFF - (ticklen < 0 ? ticklen : 0);
	else
		xl_off = -L_OFF + (ticklen < 0 ? ticklen : 0);
	if(urx - yor_x < yor_x - llx)	/* y axis in right half */
		yl_off = L_OFF - (ticklen < 0 ? ticklen : 0);
	else
		yl_off = -L_OFF + (ticklen < 0 ? ticklen : 0);

/* draw plot title if given */
	if(p_title != NULL && *p_title != '\0') {
		yv = ury + L_OFF + T_OFF - (ticklen < 0 ? ticklen : 0);
		(void)move(llx + (urx - llx) / 2, yv);
		(void)alabel('c', 'b', p_title);
	}

/* draw x axis title if given */
	if(x_title != NULL && *x_title != '\0') {
		xval = x_spc * floor(x_max / x_spc);
		yv = xor_y + xl_off - (xl_off < 0 ? T_OFF : -T_OFF);
		(void)move(xcvt(ctm, xval - x_spc * 0.5, 0.0), yv);
		(void)alabel('c', 'c', x_title);
	}

/* draw y axis title if given */
	if(y_title != NULL && *y_title != '\0') {
		yval = y_spc * floor(y_max / y_spc);
		xv = yor_x + yl_off - (yl_off < 0 ? T_OFF : -T_OFF);
		(void)move(xv, ycvt(ctm, 0.0, yval - y_spc * 0.5));
/*		(void)rotate(90);
*/		(void)alabel('c', 'c', y_title);
/*		(void)rotate(0);
*/	}

	if(grid == GRIDNONE)
		return(0);

/* draw tick marks or grid lines */
	(void)linemod(grid == GRIDFULL ? "dotted" : "solid");

	xt_len = xl_off < 0 ? ticklen : -ticklen;
	i0 = (int)floor(x_min / x_spc);
	i1 = (int)ceil(x_max / x_spc);
	for(i = i0; i <= i1; i++) {
		xval = x_spc * i;
		xv = xcvt(ctm, xval, 0.0);
		if(xv >= llx && xv <= urx) {
			if(!(nolabels & X_AXIS || (!i && grid==GRIDORIGAXES))) {
				(void)move(xv, xor_y + xl_off);
#ifdef TS_HACK
				if (ts_x){
					ts_jdate(&tm, xval);
					(void)strftime(lbl, 512, tfmt, &tm);
				} else
					(void)sprintf(lbl, "%g", log_axis & X_AXIS ?
					pow(10.0, xval) : xval);
#else
				(void)sprintf(lbl, "%g", log_axis & X_AXIS ?
					pow(10.0, xval) : xval);
#endif
				(void)alabel('c', xl_off < 0 ? 't' : 'b', lbl);
			}
			switch(grid) {
			case GRIDFULL:
				(void)line(xv, lly, xv, ury);
				break;
			case GRIDALLTICKS:
				(void)line(xv, ury, xv, ury - xt_len);
				/* FALLTROUGH */
			default:
				(void)line(xv, xor_y, xv, xor_y + xt_len);
				break;
			}
		}
		if(!(log_axis & X_AXIS) || (x_spc < 0.0 ? i == i0 : i == i1))
			continue;

		/* some more ticks for logaxis */
		for(j = 2; j < 10; j++) {
			xval = log10(pow(10.0, i * x_spc) * j);
			xv = xcvt(ctm, xval, 0.0);
			if(xv < llx || xv > urx)
				continue;

			switch(grid) {
			case GRIDFULL:
				(void)line(xv, lly, xv, ury);
				break;
			case GRIDALLTICKS:
				(void)line(xv, ury, xv, ury - xt_len / 2);
				/* FALLTROUGH */
			default:
				(void)line(xv, xor_y, xv, xor_y + xt_len / 2);
				break;
			}
		}
	} /* endfor x axis */

	yt_len = yl_off < 0 ? ticklen : -ticklen;
	i0 = (int)floor(y_min / y_spc);
	i1 = (int)ceil(y_max / y_spc);
	for(i = i0; i <= i1; i++) {
		yval = y_spc * i;
		yv = ycvt(ctm, 0.0, yval);
		if(yv >= lly && yv <= ury) {
			if(!(nolabels & Y_AXIS || (!i && grid==GRIDORIGAXES))) {
				(void)move(yor_x + yl_off, yv);
				(void)sprintf(lbl, "%g", log_axis & Y_AXIS ?
					pow(10.0, yval) : yval);
				(void)alabel(yl_off < 0 ? 'r' : 'l', 'c', lbl);
			}
			switch(grid) {
			case GRIDFULL:
				(void)line(llx, yv, urx, yv);
				break;
			case GRIDALLTICKS:
				(void)line(urx, yv, urx - yt_len, yv);
				/* FALLTHROUGH */
			default:
				(void)line(yor_x, yv, yor_x + yt_len, yv);
				break;
			}
		}
		if(!(log_axis & Y_AXIS) || (y_spc < 0.0 ? i == i0 : i == i1))
			continue;

		/* some more ticks for logaxis */
		for(j = 2; j < 10; j++) {
			yval = log10(pow(10.0, i * y_spc) * j);
			yv = ycvt(ctm, 0.0, yval);
			if(yv < lly || yv > ury)
				continue;

			switch(grid) {
			case GRIDFULL:
				(void)line(llx, yv, urx, yv);
				break;
			case GRIDALLTICKS:
				(void)line(urx, yv, urx - yt_len / 2, yv);
				/* FALLTROUGH */
			default:
				(void)line(yor_x, yv, yor_x + yt_len / 2, yv);
				break;
			}
		}
	} /* endfor y axis */

/* draw the box/axes */
	if(grid == GRIDFULL)
		(void)linemod("solid");

	if(grid == GRIDFULL || grid == GRIDALLTICKS)
		(void)box(llx, lly, urx, ury);
	else {
		(void)line(yor_x, lly, yor_x, ury);
		(void)line(llx, xor_y, urx, xor_y);
	}
	return(0);
}
