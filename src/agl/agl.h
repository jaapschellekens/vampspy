/* %W%  (OAA-ASTRONET) %G% %U%   */
/*
 * HEADER : agl.h          - Vers 3.6.001  - Sep 1993 -  L. Fini, OAA
 *                         - Vers 3.6.000  - Nov 1991 -  L. Fini, OAA
 */

/****************************************************************************/
/*              Prototypes for AGL application programs           */

#ifndef ANSI

#ifdef ansi
#define ANSI
#endif

#ifdef __STDC__
#define ANSI
#endif

#endif

void   AG_AXES(
#ifdef ANSI
		double, 
		double, 
		double, 
		double, 
		char *
#endif
);

void   AG_AXIS(
#ifdef ANSI
		int,
		float[],
		double,
		char *,
		char *
#endif
);

void   AG_CDEF(
#ifdef ANSI
		double,
		double,
		double,
		double
#endif
);


void   AG_CLS(
#ifdef ANSI
		void
#endif
);

void   AG_DMSG(
#ifdef ANSI
		char *,
		char *
#endif
);

void   AG_DRIV(
#ifdef ANSI
		void
#endif
);

void   AG_ESC(
#ifdef ANSI
		char *,
		int
#endif
);

void AG_FILL(
#ifdef ANSI
	float [],
	float [],
	int,
	double,
	double,
	char *
#endif
);

void   AG_GERR(
#ifdef ANSI
		int,
		char *
#endif
);

char * AG_GETN(
#ifdef ANSI
		char *,
		int,
		FILE *,
		int *
#endif
);

char * AG_GETS(
#ifdef ANSI
		char *,
		int,
		FILE *
#endif
);

void   AG_GINT(
#ifdef ANSI
		float [],
		float [],
		int
#endif
);

void   AG_GPLG(
#ifdef ANSI
		float [],
		float [],
		int
#endif
);

void   AG_GPLL(
#ifdef ANSI
		float [],
		float [],
		int
#endif
);

void   AG_GPLM(
#ifdef ANSI
		float [],
		float [],
		int,
		int
#endif
);

void   AG_GTXT(
#ifdef ANSI
		double,
		double,
		char *,
		int
#endif
);

char * AG_IDN(
#ifdef ANSI
		void
#endif
);

int    AG_IGET(
#ifdef ANSI
		char *,
		int []
#endif
);

void   AG_ISET(
#ifdef ANSI
		int,
		int []
#endif
);

int    AG_IVAL(
#ifdef ANSI
		char *,
		int,
		int []
#endif
);

void   AG_HIST(
#ifdef ANSI
		float [],
		float [],
		int,
		int,
		int
#endif
);

void AG_MAGN (
#ifdef ANSI
		double,
		double,
		float [],
		float [],
		int
#endif
);

void   AG_MCLS(
#ifdef ANSI
		void
#endif
);

void   AG_MOPN(
#ifdef ANSI
		char *
#endif
);

void   AG_MRDW(
#ifdef ANSI
		char *
#endif
);

void   AG_MRES(
#ifdef ANSI
		void
#endif
);

void   AG_MSUS(
#ifdef ANSI
		void
#endif
);

void   AG_NEWN(
#ifdef ANSI
		char *
#endif
);

void AG_NLIN(
#ifdef ANSI
		double,
		double,
		double,
		double,
		char *
#endif
);

void AG_ORAX(
#ifdef ANSI
		int,
		float *,
		float *,
		char *,
		char *
#endif
);

int    AG_RGET(
#ifdef ANSI
		char *,
		float []
#endif
);

void   AG_RSET (
#ifdef ANSI
		int,
		float []
#endif
);

int    AG_RVAL(
#ifdef ANSI
		char *,
		int,
		float []
#endif
);

char * AG_SCAN(
#ifdef ANSI
		char *,
		int,
		int,
		char *
#endif
);

void   AG_SSET(
#ifdef ANSI
		char *
#endif
);

#ifndef UXCFINTF

FILE * AG_STDO(
#ifdef ANSI
		char *,
		char *,
		int
#endif
);

#endif

int    AG_SVAL(
#ifdef ANSI
		char *,
		int,
		char *
#endif
);

void   AG_TGET(
#ifdef ANSI
		char *,
		float [],
		float []
#endif
);

void   AG_TRNS (
#ifdef ANSI
		void (*)(),
		int (*)(),
		int (*)()
#endif
);


void AG_TROT (
#ifdef ANSI
		float [],
		float [],
		int
#endif
);

void AG_TSET (
#ifdef ANSI
		double,
		double,
		double,
		int
#endif
);

int    AG_VDEF(
#ifdef ANSI
		char *,
		double,
		double,
		double,
		double,
		double,
		double
#endif
);

void   AG_VERS(
#ifdef ANSI
		void
#endif
);

void   AG_VKIL(
#ifdef ANSI
		void
#endif
);

void   AG_VLOC(
#ifdef ANSI
		float *,
		float *,
		int *,
		int *
#endif
);

void   AG_VLOS(
#ifdef ANSI
		float *,
		float *,
		int,
		char *,
		int *
#endif
);

void   AG_VN2U(
#ifdef ANSI
		double,
		double,
		float *,
		float *
#endif
);

void   AG_VSEL(
#ifdef ANSI
		int
#endif
);

void   AG_VU2N(
#ifdef ANSI
		double,
		double,
		float *,
		float *
#endif
);

void   AG_VUPD(
#ifdef ANSI
		void
#endif
);

void   AG_WDEF(
#ifdef ANSI
		double,
		double,
		double,
		double
#endif
);




				/* Define marker names			*/
#define DOT			0
#define EXAGON			1
#define BOX			2
#define TRIANGLE		3
#define PLUS			4
#define CROSS			5
#define CROSS_ON_PLUS		6
#define STAR			7
#define PLUS_ON_BOX		8
#define CROSS_ON_BOX		9
#define LOZENGE			10
#define HBAR			11
#define VBAR			12
#define RIGHT_ARROW		13
#define UP_ARROW		14
#define LEFT_ARROW		15
#define DOWN_ARROW		16
#define FILLED_EXAGON		17
#define FILLED_BOX		18
#define FILLED_TRIANGLE		19
#define FILLED_LOZENGE		20
