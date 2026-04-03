/* $Header: /home/schj/src/vamps_0.99g/src/main/RCS/sigs.c,v 1.8 1999/01/06 12:13:01 schj Alpha $ 
 */
/*  $RCSfile: sigs.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

#ifdef HAVE_CONFG_H
#include "vconfig.h"
#endif

static char RCSid[] =
"$Id: sigs.c,v 1.8 1999/01/06 12:13:01 schj Alpha $";

#include <stdio.h>
#include <signal.h>
#include "vamps.h"
#include "deffile.h"

int    setsig (void);
int    unsetsig (void);
int    switchtoint=0;

/* Number of signals to be ignored */
#define MAXIG 1
#ifdef unix
#define CATCHSIGS SIGINT, SIGABRT, SIGTERM, SIGFPE, SIGHUP, SIGWINCH, SIGPIPE, SIGSEGV, SIGALRM
#else
#define CATCHSIGS SIGINT, SIGABRT, SIGTERM, SIGFPE, SIGHUP
#endif

/* Signals to be caught or ignored */
const int catch[] =
{CATCHSIGS, 0};

/* Needed in djgpp */
#ifndef SIG_ERR
#define SIG_ERR 1
#endif

void
exitfunc()
{
	/* delete all datasets and free memory  */
	del_all_sets ();
	/* deletes the ini memory list (if present)  */
	delmemlist ();
}

/*C:onsig
 *@void onsig (int sig)
 *
 * New signal handler. Setup is run again after handling the signal as
 * Linux resets them to default behaviour. I suppose this is no
 * problem on other systems ;-).
 * Under MSDOS (djgpp) the stuff does not seem to work (what's new). */
#define EMG 2
extern FILE *gp_cmd_file;
void
onsig (int sig)
{
	char s[512];

	switch (sig){
	default:
		(void)sprintf(s,"\nExiting on signal %d", sig);
		(void) showit ("sig",WARN,s,0,0);
		/* perror("vamps ");*/
		exit (sig);
		break;
	}
}

/*C:setsig
 *@void setsig()
 *
 * set up custom signal handling. Works on UNIX systems, rather broken
 * on other systems */
int
setsig (void)
{
	int     err = 0;
	int     i;

	/* set up signal handler */
	for (i = 0; catch[i] && !err; i++)
		if (signal (catch[i], onsig) == SIG_ERR){
			showit("sig",WARN,"Can't set up signal handler",0,0);
			Perror (progname, 0,1, RCSid, "Can't set up signal handler", "");
			err++;
		}
	return err;
}

/*C:unsetsig
 *@int unsetsig()
 *
 * resets signal handling
 */
int
unsetsig ()
{
	int     err = 0;
	int     i;

	/* resets signal handler */
	for (i = 0; catch[i] && !err; i++)
		if (signal (catch[i], SIG_DFL) == SIG_ERR) {
			showit("sig",WARN,"Can't reset signal handler",0,0);
			Perror (progname, 0,1, RCSid, "Can't reset signal handler", "");
			err++;
		}
	return err;
}
