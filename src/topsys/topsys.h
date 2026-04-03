/* $Header: /home/schj/src/vamps_0.99g/src/topsys/RCS/topsys.h,v 1.3 1999/01/06 12:13:01 schj Alpha $ */
/* 
 *  $RCSfile: topsys.h,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

#ifdef DEBUG
static char RCSid[] =
"$Id: topsys.h,v 1.3 1999/01/06 12:13:01 schj Alpha $";
#endif

/* Topsys functions to be called from main() */

extern void init_top(int toptype);
extern void (*tstep_top) (int tstep, double *precipitation,
		double *interception, double *transpiration,
		double *soilevaporation);
extern void (*pre_top) ();
extern void (*post_top) ();

#define TOP_NOOP 0	/* EMPTY SYSTEM */
#define TOP_SOIL 1	/* bare soil */
#define TOP_FUL_CANOP 2	/* full canopy */
#define TOP_PAR_CANOP 3	/* partial canopy */
#define TOP_SCRIPT 4	/* scripting language base top-system */
#define TOP_PRE_CANOP 5	/* all canopy stuff precalculated */
#define TOP_OCANOP 6	/* old (0.99b) canopy.c */

typedef struct {
	unsigned int	id; /* is actually redundant ...*/
	char	desc[128];
	void (*tstep_top) (int tstep, double *precipitation,
				double *interception, double *transpiration,
				double *soilevaporation);
	void (*pre_top) ();
	void (*post_top) ();
} ttop;

#define NRTOPSYS (sizeof  toptype / sizeof toptype[0])
