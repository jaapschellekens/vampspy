/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/progress.c,v 1.25 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: progress.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static char RCSid[] =
"$Id: progress.c,v 1.25 1999/01/06 12:13:01 schj Alpha $";

#include "swatsoil.h"
#include "vamps.h"
#include "deffile.h"
#include <stdio.h>
#include <errno.h>

#include <time.h>
#ifndef __MSDOS__
#include <unistd.h>
#endif

/* int fmttime = TRUE; */

#define POINTSTOSHOW 413

#ifdef __OS2__
#define TMPTHETA "c:\\tmp\\the.tmp"
#else
#define TMPTHETA "/tmp/_the._tmp"
#endif

#ifdef HAVE_POPEN
extern FILE *popen (const char *command, const char *type);
extern int pclose (FILE *stream);
#endif

#define TIMEFMT "%x %X"

#define PROGSTR1 "\
-\b\\\b|\b/\b--\b\\\b|\b/\b--\b\\\b|\b/\b--\b\\\b|\b/\b--\b\\\b|\b/\b-\
-\b\\\b|\b/\b--\b\\\b|\b/\b--\b\\\b|\b/\b--\b\\\b|\b/\b--\b\\\b|\b/\b-\
"

#define PROGSTR2 "\
0%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
10%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
20%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
30%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
40%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
50%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
60%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
70%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
80%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
90%\
-\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b--\b/\b|\b\\\b-\
100%"


char progstr[2048];
int ptpst;
int ptt;
int rest;
int tpoints = 0;
int pointstoshow = POINTSTOSHOW;
static time_t t_starttime;
static time_t nowtime;
static int showtime;
int slprog = 0; /* show progress though slang_function (was S-Lang, now disabled) */


/*-
 * void initprogress()
 * initializes the progress bar system
 -*/
void
initprogress ()
{
	int i;

	t_starttime = time (NULL);
	strcpy (progstr, "3");
	if (pointstoshow == POINTSTOSHOW)	/* allways TRUE */{
		{ const char *_t = getdefstr ("vamps", "progstr", progstr, infilename, FALSE);
		  if (_t != progstr) strncpy(progstr, _t, 2047); }
		if (strcmp (progstr, "1") == 0)
		  strcpy (progstr, PROGSTR1);
		if (strcmp (progstr, "2") == 0)
		  strcpy (progstr, PROGSTR2);
		if (strcmp (progstr, "3") == 0)
		  showtime = 1;
		pointstoshow = strlen (progstr);
	} else  {
		for (i = 0; i < pointstoshow; i++){
		    progstr[i] = '.';
		}
		progstr[pointstoshow] = '\0';
	}
	ptpst = steps / pointstoshow == 0 ? 1 : steps / pointstoshow;
	ptt = pointstoshow / steps == 0 ? 1 : pointstoshow / steps;
	rest = pointstoshow % steps;
}

/*-
 * void showprogress(int step)
 *	shows a progress bar
 -*/

void
showprogress (int step)
{
	int pt = 0;
	static int firstcall=1;
	char tmstr[1024];
	int ret;
	double diff, togo;
	
	strcpy(tmstr,"");
	if (showtime){
		if (firstcall == 1){
			(void) fprintf (stderr, "\n\n\n\n+-----------------------------------------------------------------------------+\n");
			(void) fprintf (stderr, "| time              |  running | to_go |   [masbal, it,   err, dt,water_cont] |\n");
			(void) fprintf (stderr, "+-----------------------------------------------------------------------------+\n");
			firstcall=0;
		}
		nowtime = time (NULL);
		diff = /* difftime (nowtime, t_starttime);*/ (double)(nowtime-t_starttime);
		togo = ((diff * (double) steps / (double) step) - diff);
		
		sprintf(tmstr,"%17.6f",t);
		if (diff > 3600 || togo > 3600)
		  fprintf (stderr, "|%s  |%5.2f hrs.|%5.2f  |   [%6.3f,%3d,%5d,%8.6f,%5.3f] |\r", tmstr, diff / 3600, (togo) / 3600, masbal, itter[step - 1], sol_error[step - 1], dt, volact);
		else if (diff > 60 || togo > 60)
		  fprintf (stderr, "|%s  |%5.2f min.|%5.2f  |   [%6.3f,%3d,%5d,%8.6f,%5.3f] |\r", tmstr, diff / 60, (togo) / 60, masbal, itter[step - 1], sol_error[step - 1], dt, volact);
		else
		  fprintf (stderr, "|%s  |%5.0f sec.|%5.0f  |   [%6.3f,%3d,%5d,%8.6f,%5.3f] |\r", tmstr, diff, (togo), masbal, itter[step - 1], sol_error[step - 1], dt, volact);
		
		if (steps == t)
		  fprintf (stderr, "\n");
	}else{
		if (step % ptpst == 0 && (step / ptpst) * ptt <= pointstoshow)
		  do{
			  fprintf (stderr, "%c", progstr[tpoints]);
			  pt++;
			  tpoints++;
		  }while (pt < ptt);
		
		if (step == steps && ptt > 1)
		  for (pt = 0; pt < rest; pt++)	{
			  fprintf (stderr, "%c", progstr[tpoints]);
			  tpoints++;
		  }
	}
}
