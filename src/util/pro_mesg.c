/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/pro_mesg.c,v 1.19 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: pro_mesg.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static char RCSid[] =
"$Id: pro_mesg.c,v 1.19 1999/01/06 12:13:01 schj Alpha $";


#include <stdio.h>
#include <time.h>
#include "vamps.h"
#include "version.h"
#include "swatsoil.h"
#include "deffile.h"

char logfilename[1024];
int verbose = FALSE;
int loggen = FALSE;
static FILE *showfile = NULL;
static FILE *logfile = NULL;
/*static FILE *logfile; */
char deffname[1024];

int nrwarn = 0;
int nrmesg = 0;
int nrerr = 0;

/* Forward declarations */
void startlog (char *prname, char *name);
int endlog(void);

/*-
 * void logit(char *tolog,char *from,char *TYPE)
 *
 * Logs to logfile
 */
void
logit (char *tolog,char *from,char *TYPE)
{
  	if (logfile)
  		(void) fprintf (logfile, "%s: %s: %s: %f: %d\n",from, TYPE,tolog,t,thisstep);
}

/*C:showit
 *@ void showit (char *from,char *type, char *toshow,
 *               int atverb,int whichverb)
 *
 * prints stuff to showfile if whichverb >= atverb
 */
void
showit (char *from,char *type, char *toshow,int atverb,int whichverb)
{
  if (whichverb >= atverb)
    if (!fprintf (showfile, "%s: %s: %s: %f:%d\n",from,type,toshow,t,thisstep))
      Perror (progname,1,1, RCSid, "Could not write to showfile", toshow);

if (strcmp(MESG,type) == 0)
	nrmesg++;
else if (strcmp(WARN,type) == 0)
	nrwarn++;
else if (strcmp(ERR,type) == 0)
	nrerr++;

  if (loggen)
    logit (toshow,from,type);
}

void
startshow (char *prname,char *name)
{
  char s[1024];
  time_t tt;
  struct tm *tm;
  int wrote = 0;

  if (showfile == NULL)
     showfile = stderr;
   
  if (verbose){
    tt = time (NULL);
    tm = localtime (&tt);
    (void) strftime (s, (size_t) 1024, "%A %B %d, %Y, %H:%M.%S", tm);
    wrote += fprintf (showfile, "\n%c %s:\n%c\t%s\n%c\t%s %s\n%c\t%s (c) %s\n", commchar[0], prname, commchar[0], DESCRIPTION, commchar[0], IDENTIFY, PROVERSION, commchar[0], AUTHOR, DATE);
    wrote += fprintf (showfile, "%c\tBuild on %s at %s by %s\n", commchar[0], WHERE, BUILDDATE, WHO);
    wrote += fprintf (showfile, "\n%c Run starting at: %s\n", commchar[0], s);
    wrote += fprintf (showfile, "%c Using input from: %s\n", commchar[0], name);
    wrote += fprintf (showfile, "%c Using defaults from: %s\n", commchar[0], deffname);
    if (wrote < 15)
      Perror (progname,1,1, RCSid, "Could not write to showfile", "startshow()");
  }
  if (loggen)
    startlog(prname,name);
}

int
endlog (void)
{
  char s[1024];
  time_t tt;
  struct tm *tm;
  int wrote;

  tt = time (NULL);
  tm = localtime (&tt);
  printsum(logfile);
  (void) fprintf(logfile,"pro_mesg: mesg: number of messages: %d: %f %d\n",nrmesg+2,t,thisstep);
  (void) fprintf(logfile,"pro_mesg: mesg: number of warnings: %d: %f %d\n",nrwarn,t,thisstep);
  (void) fprintf(logfile,"pro_mesg: mesg: number of errors: %d: %f %d\n",nrerr,t,thisstep);
  (void) strftime (s, (size_t) 1024, "%A %B %d, %Y, %H:%M.%S", tm);
  wrote = fprintf (logfile, "\n%c Run ending at: %s\n", commchar[0], s);
  return fclose(logfile);
}

void
endshow ()
{
  char s[1024];
  time_t tt;
  struct tm *tm;
  int wrote;

  tt = time (NULL);
  tm = localtime (&tt);
  if (verbose){ 
  (void) fprintf(showfile,"pro_mesg: mesg: number of messages: %d: %f %d\n",nrmesg+2,t,thisstep);
  (void) fprintf(showfile,"pro_mesg: mesg: number of warnings: %d: %f %d\n",nrwarn,t,thisstep);
  (void) fprintf(showfile,"pro_mesg: mesg: number of errors: %d: %f %d\n",nrerr,t,thisstep);
    (void) strftime (s, (size_t) 1024, "%A %B %d, %Y, %H:%M.%S", tm);
    wrote = fprintf (showfile, "\n%c Run ending at: %s\n", commchar[0], s);
  }
  if (loggen)
    (void)endlog();
}

void
startlog (char *prname, char *name)
{
 char s[1024];
  time_t tt;
  struct tm *tm;
  int wrote = 0;

  if((logfile = fopen(logfilename,"w")) == NULL)
    Perror(prname,1,1,RCSid,"Unable to open logfile","logfilename");
  tt = time (NULL);
  tm = localtime (&tt);
  (void) strftime (s, (size_t) 1024, "%A %B %d, %Y, %H:%M.%S", tm);
  wrote += fprintf (logfile, "\n%c %s:\n%c\t%s\n%c\t%s %s\n%c\t%s (c) %s\n", commchar[0], prname, commchar[0], DESCRIPTION, commchar[0], IDENTIFY, PROVERSION, commchar[0], AUTHOR, DATE);
  wrote += fprintf (logfile, "%c\tBuild on %s at %s by %s\n", commchar[0], WHERE, BUILDDATE, WHO);
  wrote += fprintf (logfile, "\n%c Run starting at: %s\n", commchar[0], s);
  wrote += fprintf (logfile, "%c Using input from: %s\n", commchar[0], name);
  wrote += fprintf (logfile, "%c Using defaults from: %s\n", commchar[0], deffname);
  if (wrote < 15)
    Perror (progname,1,1, RCSid, "Could not write to logfile", "startlog()");
}

void
outfileheader (char *name)
{
  char s[1024];
  time_t tt;
  struct tm *tm;
  int wrote = 0;

  tt = time (NULL);
  tm = localtime (&tt);
  (void) strftime (s, (size_t) 1024, "%A %B %d, %Y, %H:%M.%S", tm);
  wrote += fprintf (genoutfile, "\n%c %s:\n%c\t%s\n%c\t%s %s\n%c\t%s (c) %s\n", commchar[0], progname, commchar[0], DESCRIPTION, commchar[0], IDENTIFY, PROVERSION, commchar[0], AUTHOR, DATE);
  wrote += fprintf (genoutfile, "%c\tBuild on %s at %s by %s\n", commchar[0], WHERE, BUILDDATE, WHO);
  wrote += fprintf (genoutfile, "\n%c Run starting at: %s\n", commchar[0], s);
  wrote += fprintf (genoutfile, "%c Using input from: %s\n", commchar[0], name);
  wrote += fprintf (genoutfile, "%c Using defaults from: %s\n", commchar[0], deffname);
  wrote += fprintf (genoutfile, "\n%c Use the vsel command to extract information from this file.\n", commchar[0]);
  wrote += fprintf (genoutfile, "%c The input values are dumped at the end of this file.\n", commchar[0]);
  wrote += fprintf (genoutfile, "%c Thank you for using %s!\n", commchar[0], progname);
  wrote += fprintf (genoutfile, "\n[header]\nrun_start_time= %s\n", s);
  if (wrote < 15)
    Perror (progname,1,1, RCSid, "Could not write to outfile", "outfileheader()");

}

void
outfiletrailer ()
{
  char s[1024];
  time_t tt;
  struct tm *tm;
  int wrote;

  tt = time (NULL);
  tm = localtime (&tt);
  (void) strftime (s, (size_t) 1024, "%A %B %d, %Y, %H:%M.%S", tm);
  wrote = fprintf (genoutfile, "\n[trailer]\nrun_end_time= %s\n", s);
}
