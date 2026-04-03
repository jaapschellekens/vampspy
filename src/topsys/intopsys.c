/* $Header: /home/schj/src/vamps_0.99g/src/topsys/RCS/intopsys.c,v 1.4 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: intopsys.c,v $
 * $Author: schj $
 * $Date: 1999/01/06 12:13:01 $ */

#include "topsys.h"
#include "vamps.h"
#ifdef HAVE_LIBPYTHON
#include "py_init.h"
#endif


static char RCSid[] =
 "$Id: intopsys.c,v 1.4 1999/01/06 12:13:01 schj Alpha $";

/* forward declarations of dummy functions */
void tstep_top_noop (int tstep, double *precipitation,
		double *interception, double *transpiration,
		double *soilevaporation);
void pre_top_noop();
void post_top_noop();

/* These should be linked to the above*/
/* no-trees (or plants) setup */
extern void tstep_top_notree (int tstep, double *precipitation,
		double *interception, double *transpiration,
		double *soilevaporation);
extern void pre_top_notree (void);
extern void post_top_notree (void);

/* Note: Python topsys functions are declared in py_init.h and included above */

/* Topsys in which all fluxes ar read from a file */
extern void tstep_top_pre_canop (int tstep, double *precipitation,
		double *interception, double *transpiration,
		double *soilevaporation);
extern void pre_top_pre_canop (void);
extern void post_top_pre_canop (void);

/* 'Old 0.99b topsys (canopy.c) */
extern void precanop (void);
extern void postcanop (void);
extern void tstep_ocanop (int tstep, double *precipitation,
		double *interception, double *transpiration,
		double *soilevaporation);


/* Functions to call */
void (*tstep_top) (int tstep, double *precipitation,
		double *interception, double *transpiration,
		double *soilevaporation) = NULL;
void (*pre_top) () = NULL;
void (*post_top) () = NULL;

/* 
   Usage:
   ID, "Description", tstep func, pre func, post func
*/
ttop	toptype[] = {
	{TOP_NOOP, "Empty topsystem",
		tstep_top_noop,pre_top_noop,post_top_noop
	},
	{TOP_SOIL, "Bare soil",
		tstep_top_notree,pre_top_notree,post_top_notree
	},
	{TOP_FUL_CANOP, "Full canopy",
		/*tstep_top_fcanop,pre_top_fcanop,post_top_fcanop*/
		tstep_top_noop,pre_top_noop,post_top_noop
	},
	{TOP_PAR_CANOP, "Partial canopy",
		/*tstep_top_pcanop,pre_top_pcanop,post_top_pcanop*/
		tstep_top_noop,pre_top_noop,post_top_noop
	},
	{TOP_PRE_CANOP, "All canopy stuff precalculated",
		tstep_top_pre_canop,pre_top_pre_canop,post_top_pre_canop
	},
	{TOP_OCANOP, "Old canopy.c topsystem",
		tstep_ocanop,precanop,postcanop
	}
#ifdef HAVE_LIBPYTHON
	,
	{TOP_SCRIPT, "Python scripted top-system",
		py_tstep_top_script,py_pre_top_script,py_post_top_script
	}
#endif
};
static int nrtopsys = NRTOPSYS;

void 
tstep_top_noop (int tstep, double *precipitation,
		double *interception, double *transpiration,
		double *soilevaporation)
{
	(void)fprintf(stderr,"Topsystem not yet finished\n");
	exit(1);
}

void
pre_top_noop()
{
	(void)fprintf(stderr,"Topsystem not yet finished\n");
	exit(1);
}

void
post_top_noop()
{
	(void)fprintf(stderr,"Topsystem not yet finished\n");
	exit(1);
}


/*C:init_top
 *@void init_top(int topt)
 * Initializes the topsystem by pointing @tstep_top@, @pre_top@
 * and @post_top@ to the functions determined by @toptype@.
 *
 * Values for topt in topsys.h
 */ 
void
init_top(int topt)
{
	char s[128];
	void prtopsys();

	if (getsetbyname("inr") == -1)
		(void)add_set (NULL, "inr", "inr", steps, 0, 0);
	if (getsetbyname("spe") == -1)
		(void)add_set (NULL, "spe", "spe", steps, 0, 0);
	if (getsetbyname("ptr") == -1)
		(void)add_set (NULL, "ptr", "ptr", steps, 0, 0);
	if (getsetbyname("trf") == -1)
		(void)add_set (NULL, "trf", "trf", steps, 0, 0);
	if (getsetbyname("stf") == -1)
		(void)add_set (NULL, "stf", "stf", steps, 0, 0);
	id.inr = getsetbyname("inr");
	id.spe = getsetbyname("spe");
	id.ptr = getsetbyname("ptr");
	id.trf = getsetbyname("trf");
	id.stf = getsetbyname("stf");

	if (topt < 0 || topt >= nrtopsys){
		prtopsys();
		Perror(progname,1,0,RCSid,"Invalid topsystem given","See list above");
	}
	sprintf(s,"%d topsystems, using %d (%s)\n",nrtopsys,topt,toptype[topt].desc);	
	showit("topsys",MESG,s,1,verbose);
	tstep_top = toptype[topt].tstep_top;
	pre_top =  toptype[topt].pre_top;
	post_top = toptype[topt].post_top;
}

/*C:prtopsys
 *@ void prtopsys(void)
 *
 * Prints all topsystems descriptions on @stderr@
 */
void
prtopsys()
{
	int	i;

	for (i = 0; i < nrtopsys; i++){
		(void)fprintf(stderr,"%d: %s\n",i,toptype[i].desc);
	}
}
