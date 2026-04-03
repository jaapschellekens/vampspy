/* $Header: /home/schj/src/vamps_0.99g/src/nr_ut.lib/RCS/log.c,v 1.3 1999/01/06 12:13:01 schj Alpha $ */
 
/* $RCSfile: log.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */


static char RCSid[] =
 "$Id: log.c,v 1.3 1999/01/06 12:13:01 schj Alpha $";

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "nrutil.h"


/*F:log.c
 *
 * Simple library for printing text to multiple streams at once. Each
 * stream can have it's own verbose level (i.e. only messages below
 * that level are printed to that stream. See @l_addf@, @l_addfn@,
 * @l_printf@, @l_error@, @l_perror@. */


/*C:l_verb
 *@ int l_verb = 0
 * Value added to verbose level of each file */
int l_verb = 0; /* added to all verbose levels */

/*C:l_switch
 *@int l_switch = 0
 * 
 * If set to non-zero the @l_switchto@ function is called in stead of
 * exiting to the system */
int l_switch = 0;

/*C:l_switchto
 * @void (* l_switchto)(int )
 * 
 * Function to run in stead of exiting to system when an error function is
 * called. Only if @l_switch@ is non zero. To use define a void function
 * which accepts an integer argument (not used at present) and insert the
 * following statement in your program: 
 *@        l_switchto = my_function;
 */
void (* l_switchto)(int ) = l_def_switch;


#define L_PREF_LENGTH 128
static char l_prefix[L_PREF_LENGTH] = "";
#define MAXFL 12


l_fltype l_fl[MAXFL + 1] = {
		{NULL,0}};

static int l_nr = 0;

void l_def_switch(int arg) /* does nothing */
{
	l_error(0,"l_def_switch",
	"set the l_switchto function to something usefull!\n(%s)\n",RCSid);
}

/*C:l_setprefix
 *@ void l_setprefix(char *pref)
 *
 * Set the prefix for messages printed by @l_error@ and @l_perror@.  */
void
l_setprefix (char *pref)
{
#ifdef HAVE_SNPRINTF
	(void)snprintf(l_prefix,L_PREF_LENGTH,"%s: ",pref);
#else
	(void)sprintf(l_prefix,"%s: ",pref);
#endif
}


/*C:l_addfn
 *@int l_addfn(int verb, char *fn)
 * 
 * Open the file with name @fn@ and add it to this list of files used by
 * the error functions. @verb@ sets the verbose level for this stream.
 * Return the number of the file or -1 on error.
 */ 
int l_addfn(int verb,char *fn)
{
	l_nr++;

	if (l_nr < MAXFL){
		if((l_fl[l_nr - 1].fl = fopen(fn,"w")) == NULL){
			l_nr--;
			return -1;
		}
		l_fl[l_nr].fl = NULL;
		l_fl[l_nr -1].verb = verb;
		return l_nr -1;
	}else
		return -1;
}

/*C:l_addf
 *@int l_addf(int verb, FILE *fp)
 * 
 * Add the open stream @fp@ to this list of files used by the error
 * functions. @verb@ sets the verbose level for this stream. Return the
 * number of the file or -1 on error.
 */ 
int l_addf(int verb,FILE *fp)
{
	l_nr++;

	if (l_nr < MAXFL){
		l_fl[l_nr - 1].fl = fp;
		l_fl[l_nr - 1].verb = verb;
		l_fl[l_nr].fl = NULL;
		return l_nr -1;
	}else
		return -1;
}

/*C:l_closeall
 *@void l_closeall(void)
 * 
 * Close all streams added with @add_fn@ and @add_f@.
 */ 

void l_closeall()
{
	int i=0;

	for (i=0; i < MAXFL;i++)
		if (l_fl[i].fl)
			fclose(l_fl[i].fl);
}

void l_werror(char *name)
{
	int i = 0;

	while (l_fl[i].fl){
		if (l_prefix)
			(void)fprintf(l_fl[i].fl,"%s",l_prefix);
		(void)fprintf(l_fl[i].fl,"%s: %s (%d)\n",name,strerror(errno),errno);
		(void)fflush(l_fl[i].fl);
		i++;
	}
}

void l_perror(char *name)
{
	int i = 0;

	l_werror(name); /* write error */

	if (!l_switch){
		l_closeall(); /* close all open files from this lib */
		exit(errno);
	}else
		l_switchto(0);
}

/*C:l_error
 *@ void l_error(int syserror, char *name, const char *fmt, ...)
 *
 * Print the formatted string (@fmt@, see @printf(3)@) to the streams
 * opened with @addf[n]@. @name@ is printed first followed by a colon
 * (:) and a space. @syseror@ indicates wether @l_perror@ (@syserror@
 * > 1), @l_werror@ (@syserror@ == 1) or no function at all
 * (@syserror@ < 1) is called after printing the message.
 *
 * See also @printf(3)@, @l_perror@, @l_werror@, @l_switch@. */

void l_error(int syserror,char *name,const char *fmt, ...)
{
	va_list args;
	int i=0;

	va_start(args, fmt);
	while (l_fl[i].fl){
		if (l_prefix)
			(void)fprintf(l_fl[i].fl,"%s",l_prefix);
		(void)fprintf(l_fl[i].fl,"%s: ",name);
		(void)vfprintf(l_fl[i].fl,fmt,args);
		(void)fflush(l_fl[i].fl);
		i++;
	}
	va_end(args);

	if (syserror > 1)
		l_perror(name);
	if (syserror == 1)
		l_werror(name);
}

int l_printf(int verb,const char *fmt, ...)
{
	va_list args;
	int i =0;

	va_start(args, fmt);
	while (l_fl[i].fl){
		if (l_fl[i].verb + l_verb >= verb){
			if (l_prefix)
				(void)fprintf(l_fl[i].fl,"%s",l_prefix);
			(void)vfprintf(l_fl[i].fl,fmt,args);
			(void)fflush(l_fl[i].fl);
		}
		i++;
	}
	va_end(args);

	return i;
}


#ifdef TESTING
main()
{
	l_addf(0,stderr);
	l_addfn(0,"testing");
	l_addfn(0,"ffff");
	if (!fopen("/ll","rw"))
		l_error(2,"vamps","vamps: could not open %s\n","/ll");
}
#endif
