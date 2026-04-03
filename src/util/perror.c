/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/perror.c,v 1.11 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: perror.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "vamps.h"
#include "deffile.h"

/*C:Perror
 *@ void Perror(char *Eprogname,int exitval,int prr, char *from,
 *		const char *descr,const char *xtr)
 *
 * If @prr@ > 0 then @perror@ is also called.
 * Prints an error message on stderr and exits with level
 * exitval if this value is > 0	
 * Normally called with something like:
 * @Perror(progname,1,RCSid,"A fatal error","divide by zero");@
 */
void
Perror (char *Eprogname,int exitval,int prr,char *from,const char *descr,const char *xtr)
{
	if (!exitval) {
		(void) fprintf (stderr, "%s:\terror message:\n", Eprogname);
		(void) fprintf (stderr, "\tfrom: %s\n", from);
		(void) fprintf (stderr, "\tdescription: %s %s\n", descr, xtr);
		if (prr)
			perror ("syserr");
	} else {
		(void) fprintf (stderr, "%s:\terror message:\n", Eprogname);
		(void) fprintf (stderr, "\tfrom: %s\n", from);
		(void) fprintf (stderr, "\tdescription: %s %s\n", descr, xtr);
		/* delete all datasets and free memory  */
		/* deletes the ini memory list (if present)  */
		if (prr)
			perror ("syserr");
		exit (exitval);
	}
}
