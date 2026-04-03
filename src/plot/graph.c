#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <plot.h>
#include "getopt.h"
#include "graph.h"
#include "version.h"

/*
 * command line specification
 */
char	*progname;	/* argv[0] */

#ifdef TS_HACK
#define OPTSTR	"m:a:bCc:g:H:h:l:n:p:r:sT:tu:w:X:x:Y:y:"
#define USAGE	"\
[--help] [--license] [--version] [-Cbst] [-a step [start]]\n\
\t[-c x|y|xy] [-g grid [frac]] [-H width] [-h frac] [-l x|y|xy]\n\
\t[-n x|y|xy] [-m fmt][-p line [symbol [size]]] [-r frac] [-T title]\n\
\t[-u frac] [-w frac] [-X xtitle] [-x min [max [spc]]] [-Y ytitle]\n\
\t[-y min [max [spc]]] [file ...]"
#else
#define OPTSTR	"a:bCc:g:H:h:l:n:p:r:sT:tu:w:X:x:Y:y:"
#define USAGE	"\
[--help] [--license] [--version] [-Cbst] [-a step [start]]\n\
\t[-c x|y|xy] [-g grid [frac]] [-H width] [-h frac] [-l x|y|xy]\n\
\t[-n x|y|xy] [-p line [symbol [size]]] [-r frac] [-T title]\n\
\t[-u frac] [-w frac] [-X xtitle] [-x min [max [spc]]] [-Y ytitle]\n\
\t[-y min [max [spc]]] [file ...]"
#endif

struct option	gr_options[] = {
	{"help",	no_argument, 0, 'h' << 8},
	{"license",	no_argument, 0, 'l' << 8},
	{"version",	no_argument, 0, 'v' << 8},
	{"backward",	no_argument, 0, 'b'},
	{"cumulative",	no_argument, 0, 'C'},
	{"savescreen",	no_argument, 0, 's'},
	{"transpose",	no_argument, 0, 't'},
#ifdef TS_HACK
	{"tsfmt",	required_argument, 0, 'm'}, /* ts_fmt strftime string */
#endif
	{"abscissae",	required_argument, 0, 'a'}, /* step [start] */
	{"clipaxis",	required_argument, 0, 'c'}, /* x|y|xy|yx */
	{"gridstyle",	required_argument, 0, 'g'}, /* style [fraction] */
	{"histogram",	required_argument, 0, 'H'}, /* binwidth */
	{"height",	required_argument, 0, 'h'}, /* fraction */
	{"logaxis",	required_argument, 0, 'l'}, /* x|y|xy|yx */
	{"no-labels",	required_argument, 0, 'n'}, /* x|y|xy|yx */
	{"plotstyle",	required_argument, 0, 'p'}, /* line [symbol [frac]] */
	{"rightshift",	required_argument, 0, 'r'}, /* fraction */
	{"title",	required_argument, 0, 'T'}, /* string */
	{"upshift",	required_argument, 0, 'u'}, /* fraction */
	{"width",	required_argument, 0, 'w'}, /* fraction */
	{"xtitle",	required_argument, 0, 'X'}, /* string */
	{"xrange",	required_argument, 0, 'x'}, /* xmin [xmax [spc]] */
	{"ytitle",	required_argument, 0, 'Y'}, /* string */
	{"yrange",	required_argument, 0, 'y'}, /* ymin [ymax [spc]] */
	{0, 0, 0, 0}
};

/* gridstyle names */
#define N_GRIDS 5
char	*grids[N_GRIDS] = {
	"none",
	"full",
	"allticks",
	"lbticks",
	"origaxes"
};

/* linemode names */
#define N_LINES	7
char	*linemodes[N_LINES] = {
	"none",
	"solid",
	"dotted",
	"dotdashed",
	"shortdashed",
	"longdashed",
	"disconnected"
};

/* symbol names */
#define N_SYMBOLS 7
char	*symbols[N_SYMBOLS] = {
	"none",
	"plus",
	"cross",
	"star",
	"box",
	"diamond",
	"circle"
};

#define VERSION "Js graph hack version 0.1"
#define RELDATE "UNRELEASED"
#define LICENSE "GNU"
void
pr_info(flg)
{
	int	i;

	if(flg == 'l' || flg == 'v')
		(void)fprintf(stderr, "%s %s (%s)\n",
			progname, VERSION, RELDATE);
	if(flg == 'l')
		(void)fprintf(stderr, "%s\n", LICENSE);
	if(flg == 'h' || flg == 'u')
		(void)fprintf(stderr, "Usage: %s %s\n", progname, USAGE);
	if(flg == 'h') {
		(void)fprintf(stderr, "GNU style options can be used:\n");
		for(i = 3; gr_options[i].name; i++) {
			(void)fprintf(stderr, "\t-%c --%-12s",
				gr_options[i].val, gr_options[i].name);
			(void)fprintf(stderr, i % 2 ? "\t" : "\n");
		}
	}
	exit(1);
}

styletonum(strp, str, lim)
char	*strp[], *str;
int	lim;
{
	int	i, len;

	len = strlen(str);
	for(i = 0; i < lim; i++)
		if(!strncmp(str, strp[i], len))
			return(i);
	if(isdigit(str[0]))
		if((i = atoi(str)) >= 0 && i < lim)
			return(i);
	return(-1);
}

argtonum(str, ret)
char	*str;
double	*ret;
{
	char	*cp;
	double	val;

	val = strtod(str, &cp);
	if(cp == str || cp[0])
		return(0);	/* not a nul terminated number */
	*ret = val;
	return(1);		/* valid number string */
}

#define WARNING(opt,arg)\
	(void)fprintf(stderr,"%s: --%s \"%s\": illegal argument ignored\n",\
	progname, opt, arg)

#ifdef TS_HACK
extern int ts_x;
extern char *tfmt;
#endif

gr_main(argc, argv)
int	argc;
char	*argv[];
{
	int	c, err, i;
	int	f_autoabs = 0;		/* option flag */
	int	f_clipaxis = 0;		/* option flag */
	int	f_cumulative = 0;	/* option flag */
	int	f_histogram = 0;	/* option flag */
	int	f_logaxis = 0;		/* option flag */
	int	f_nolabels = 0;		/* option flag */
	int	f_savescrn = 0;		/* option flag */
	int	f_transpose = 0;	/* option flag */
	int	f_xrange = 0;		/* option flag */
	int	f_yrange = 0;		/* option flag */
	char	*p_title = NULL;	/* plot title */
	char	*x_title = NULL;	/* x axis title */
	char	*y_title = NULL;	/* y axis title */
	double	ticksize = 0.02;	/* tickmark size fraction */
	double	symbolsize = 0.02;	/* symbol size fraction */
	double	fr_h = 0.8;		/* height fraction */
	double	fr_w = 0.8;		/* height fraction */
	double	fr_u = 0.1;		/* upward shift fraction */
	double	fr_r = 0.1;		/* right shift fraction */
	double	x_min, x_max;		/* x range */
	double	y_min, y_max;		/* y range */
	double	x_spc = 0.0;		/* x grid spacing */
	double	y_spc = 0.0;		/* y grid spacing */
	double	h_width = 0.0;		/* histogram binwidth */
	double	a_step = 1.0;		/* auto abscissae step */
	double	a_start = 0.0;		/* auto abscissae start */
	int	lmode = 1;		/* linemode for data */
	int	symbol = 0;		/* symbol for data */
	int	grid = GRIDFULL;	/* full grid by default */

	FILE	*fp;			/* input file ptr */
	int	tsize, ssize;		/* tick/symbol sizes */
	int	llx, lly, urx, ury;	/* plot space */
	int	np = 0;			/* number of points in array */
	Coord2	*cr2;			/* data array */

	err = 0;
	progname = argv[0];

/* parse command line */
	while((c = getopt_long(argc, argv, OPTSTR, gr_options, NULL)) != -1) {
		switch(c) {
		case 'h' << 8:	/* print help */
		case 'l' << 8:	/* print license */
		case 'v' << 8:	/* print version */
			pr_info(c >> 8);
			/* NOTREACHED */
		case 'b':	/* backward plot(5) */
			/*plot_traditional = 1;*/
			break;
		case 'C':	/* cumulative */
			f_cumulative = 1;
			break;
		case 's':	/* don't erase page */
			f_savescrn = 1;
			break;
#ifdef TS_HACK
		case 'm':	/* ts_fmt x-axis */
			ts_x = 1;
			tfmt = optarg;
			break;
#endif
		case 't':	/* transpose axes */
			f_transpose = 1;
			break;
		case 'a':	/* auto abscissae */
			if(!(f_autoabs = argtonum(optarg, &a_step)))
				WARNING("abscissae", optarg);
			else if(optind < argc
				&& argtonum(argv[optind], &a_start)) {
				f_autoabs++;
				optind++;	/* tell getopt */
			}
			break;
		case 'c':	/* clipaxis */
			while(*optarg) {
				if(*optarg == 'x' || *optarg == 'X')
					f_clipaxis |= X_AXIS;
				else if(*optarg == 'y' || *optarg == 'Y')
					f_clipaxis |= Y_AXIS;
				else
					WARNING("clipaxis", optarg);
				optarg++;
			}
			break;
		case 'g':	/* gridstyle */
			if((grid = styletonum(grids, optarg, N_GRIDS)) < 0)
				grid = GRIDFULL;
			if(optind < argc && argtonum(argv[optind], &ticksize))
				optind++;	/* tell getopt */
			break;
		case 'H':	/* histogram */
			f_histogram = 1;
			(void)argtonum(optarg, &h_width);
			break;

		case 'h':	/* plot height fraction */
			(void)argtonum(optarg, &fr_h);
			break;
		case 'l':	/* logarithmic axis */
			while(*optarg) {
				if(*optarg == 'x' || *optarg == 'X')
					f_logaxis |= X_AXIS;
				else if(*optarg == 'y' || *optarg == 'Y')
					f_logaxis |= Y_AXIS;
				else
					WARNING("logaxis", optarg);
				optarg++;
			}
			break;
		case 'n':	/* no tickmark labels */
			while(*optarg) {
				if(*optarg == 'x' || *optarg == 'X')
					f_nolabels |= X_AXIS;
				else if(*optarg == 'y' || *optarg == 'Y')
					f_nolabels |= Y_AXIS;
				else
					WARNING("no-labels", optarg);
				optarg++;
			}
			break;
		case 'p':	/* plotstyle */
			if((lmode = styletonum(linemodes, optarg, N_LINES)) < 0)
				lmode = 1;
			if(optind == argc)
				break;
			if((symbol = styletonum(symbols, argv[optind],
					N_SYMBOLS)) < 0) {
				symbol = 0;
				break;		/* probably not an arg */
			}
			if(++optind < argc
				&& argtonum(argv[optind], &symbolsize))
				optind++;	/* tell getopt */
			break;
		case 'r':	/* right shift fraction */
			(void)argtonum(optarg, &fr_r);
			break;
		case 'T':	/* plot title string */
			p_title = optarg;
			break;
		case 'u':	/* upward shift fraction */
			(void)argtonum(optarg, &fr_u);
			break;
		case 'w':	/* plot width fraction */
			(void)argtonum(optarg, &fr_w);
			break;
		case 'X':	/* x axis title string */
			x_title = optarg;
			break;
		case 'x':	/* x range */
			if(!(f_xrange = argtonum(optarg, &x_min)))
				WARNING("xrange", optarg);
			else if(optind < argc
				&& argtonum(argv[optind], &x_max)) {
				f_xrange++;
				if(++optind < argc
					&& argtonum(argv[optind], &x_spc)) {
					f_xrange++;
					optind++;	/* tell getopt */
				}
			}
			break;
		case 'Y':	/* y axis title string */
			y_title = optarg;
			break;
		case 'y':	/* y range */
			if(!(f_yrange = argtonum(optarg, &y_min)))
				WARNING("yrange", optarg);
			else if(optind < argc
				&& argtonum(argv[optind], &y_max)) {
				f_yrange++;
				if(++optind < argc
					&& argtonum(argv[optind], &y_spc)) {
					f_yrange++;
					optind++;	/* tell getopt */
				}
			}
			break;
		default:
			err = 1;
			break;
		}
	}
#ifdef DEBUG
	fprintf(stderr, "Command line options:\n");
	fprintf(stderr, "  --savescreen %d\n", f_savescrn);
	fprintf(stderr, "  --transpose  %d\n", f_transpose);
	fprintf(stderr, "  --abscissae  %d %g %g\n",
			f_abscissa, a_step, a_start);
	fprintf(stderr, "  --clipaxis   %d\n", f_clipaxis);
	fprintf(stderr, "  --cumulative %d\n", f_cumulative);
	fprintf(stderr, "  --logaxis    %d\n", f_logaxis);
	fprintf(stderr, "  --no-labels  %d\n", f_nolabels);
	fprintf(stderr, "  --title     `%s'\n", p_title ? p_title : "");
	fprintf(stderr, "  --xtitle    `%s'\n", x_title ? x_title : "");
	fprintf(stderr, "  --ytitle    `%s'\n", y_title ? y_title : "");
	fprintf(stderr, "  --histogram  %d %f\n", f_histogram, h_width);
	fprintf(stderr, "  gridstyle: %d  linemode: %d    symbol: %d\n",
			grid, lmode, symbol);
	fprintf(stderr, "  symbolsize: %.2f  ticksize: %.2f\n",
			symbolsize, ticksize);
	fprintf(stderr, "  h: %.2f  w: %.2f  bm: %.2f  lm: %.2f\n",
			fr_h, fr_w, fr_u, fr_r);
	fprintf(stderr, "  xrange: ");
	fprintf(stderr, f_xrange > 0 ? "%g " : "[%g] ", x_min);
	fprintf(stderr, f_xrange > 1 ? "%g " : "[%g] ", x_max);
	fprintf(stderr, f_xrange > 2 ? "%g\n" : "[%g]\n", x_spc);
	fprintf(stderr, "  yrange: ");
	fprintf(stderr, f_yrange > 0 ? "%g " : "[%g] ", y_min);
	fprintf(stderr, f_yrange > 1 ? "%g " : "[%g] ", y_max);
	fprintf(stderr, f_yrange > 2 ? "%g\n" : "[%g]\n", y_spc);
#endif /* DEBUG */

	if(err) {
		pr_info('u');
		/* NOTREACHED */
	}
	if(isatty(fileno(stdout))) {	/* no output to tty */
		(void)fprintf(stderr,
		    "%s: Standard output is a tty, use redirection\n", argv[0]);
		exit(1);
	}

	if(f_autoabs == 1 && f_xrange)	/* abscissae start at x_min */
		a_start = x_min;

/* read in all files */
	for(;;) {
		if(optind < argc && strcmp(argv[optind], "-"))
			fp = fopen(argv[optind] , "r");
		else
			fp = stdin;
		if(fp == (FILE *)NULL) {
			(void)fprintf(stderr, "%s: Cannot open file %s\n",
						argv[0], argv[optind]);
			exit(1);
		}
		np = getinput2(fp, &cr2, np, f_autoabs, a_start, a_step);
		if(np == -1) {
			(void)fprintf(stderr, "%s: Error reading %s\n",
				argv[0], fp == stdin ? "stdin" : argv[optind]);
			exit(1);
		}

		if(fp && fp != stdin)
			(void)fclose(fp);
		if(++optind >= argc)
			break;
	}
	if(!np)
		exit(1);

  /* cumulative y data if specified */
	if(f_cumulative) {
		for(i = 1; i < np; i++)
			if(cr2[i].x >= cr2[i-1].x)	/* same dataset */
				cr2[i].y += cr2[i-1].y;
	}
  
  /* compute ranges if not specified */
	if(f_xrange < 2 || f_yrange < 2)
		getrange(cr2, np,
			f_xrange > 0 ? (double *)NULL : &x_min,
			f_yrange > 0 ? (double *)NULL : &y_min,
			f_xrange > 1 ? (double *)NULL : &x_max,
			f_yrange > 1 ? (double *)NULL : &y_max);

	if(x_min == x_max) {
		x_min -= 1.0;
		x_max += 1.0;
	}
	if(y_min == y_max) {
		y_min -= 1.0;
		y_max += 1.0;
	}

/* check margins */
	if((fr_u <= 0.0 && x_title) || (fr_r <= 0.0 && y_title)
		|| ((fr_u + fr_h >= 1.0) && p_title))
		fprintf(stderr, "Warning: margin(s) too narrow for title\n");

/* check valid ranges for logplot */
	if((f_logaxis & X_AXIS) && (x_min <= 0)) {
		fprintf(stderr, "%s: Error: logaxis: non-positive X values\n",
				progname);
		exit(1);
	}
	if((f_logaxis & Y_AXIS) && (y_min <= 0)) {
		fprintf(stderr, "%s: Error: logaxis: non-positive Y values\n",
				progname);
		exit(1);
	}

/* if we have log axes all data refer to log10 values */
	if(f_logaxis & X_AXIS) {
		for(i = 0; i < np; i++)
			cr2[i].x = log10(cr2[i].x);	/* may set errno on 0 */
		x_min = log10(x_min);
		x_max = log10(x_max);
		x_spc = 1.0;
	}
	else if(x_spc == 0.0)
		x_spc = scale1(x_min, x_max, 5);

	if(f_logaxis & Y_AXIS) {
		for(i = 0; i < np; i++)
			cr2[i].y = log10(cr2[i].y);	/* may set errno on 0 */
		y_min = log10(y_min);
		y_max = log10(y_max);
		y_spc = 1.0;
	}
	else if(y_spc == 0.0)
		y_spc = scale1(y_min, y_max, 5);

/* set spacing sign correct for draw_frame() */
	x_spc = copysign(x_spc, x_max - x_min);
	y_spc = copysign(y_spc, y_max - y_min);

/* extend axes to nearest tick mark */
	if(!(f_clipaxis & X_AXIS)) {
		x_min = x_spc * floor(x_min / x_spc);
		x_max = x_spc * ceil(x_max / x_spc);
	}
	if(!(f_clipaxis & Y_AXIS)) {
		y_min = y_spc * floor(y_min / y_spc);
		y_max = y_spc * ceil(y_max / y_spc);
	}

/* determine plot space and start output */
	setviewport(0.0, 0.0, 1.0, 1.0);
	getviewport(&llx, &lly, &urx, &ury);
	(void)openpl();
	if(!f_savescrn)
		(void)erase();
	(void)space(llx, lly, urx, ury);
	setviewport(fr_r, fr_u, fr_r + fr_w, fr_u + fr_h);

/* sizes in plot space */
	getviewport(&llx, &lly, &urx, &ury);
	tsize = (int)(((urx - llx + ury - lly + 1) / 2) * ticksize);
	ssize = (int)(((urx - llx + ury - lly + 1) / 2) * symbolsize);

/* produce the box and grid */
	if(f_transpose) {
		f_logaxis = f_logaxis >> 1 & 1 | f_logaxis << 1 & 2;
		f_nolabels = f_nolabels >> 1 & 1 | f_nolabels << 1 & 2;
		draw_frame(grid, tsize,
			y_min, y_max, y_spc, x_min, x_max, x_spc,
			y_title, x_title, p_title, f_logaxis, f_nolabels);
	}
	else
		draw_frame(grid, tsize,
			x_min, x_max, x_spc, y_min, y_max, y_spc,
			x_title, y_title, p_title, f_logaxis, f_nolabels);

/* plot the graph data */
	setmatrix(x_min, y_min, x_max, y_max, f_transpose);
	if(!f_histogram)
		draw_graph(cr2, np, lmode, symbol, ssize);
	else
		draw_histo(cr2, np, lmode, (int)h_width);

/* we're done */
	(void)closepl();
	free((void *)cr2);

	exit(0);
}
