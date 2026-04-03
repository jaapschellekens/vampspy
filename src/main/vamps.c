/* $Header: /home/schj/src/vamps_0.99g/src/main/RCS/vamps.c,v 0.40 1999/01/06 12:13:01 schj Alpha $ */

 
/* $RCSfile: vamps.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

/* Copyright (C) 1995 Jaap Schellekens.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * (C) Jaap Schellekens
 * Faculty of Earth Sciences            |       International Institute 
 * Vrije Universiteit                   |       for Tropical Forestry
 * De Boelelaan 1085                    |       Rio Piedras
 * 1081 HV Amsterdam                    |       Puerto Rico, USA
 * The Netherlands                      |
 * E-mail: schj@geo.vu.nl               |
 * schj@xs4all.nl                       |
 * ---------------------------------------------------------------------
 * Parts of this program derived from swap.  The following is taken from
 * the swap fortran code:
 * 
 * Author : Jan G. Wesseling
 * 
 * Correspondence
 * 
 * This program uses the ideas and experiences of various
 * researchers at the Winand Staring Centre and the Wageningen
 * Agricultural University. Currently the program is maintained
 * and documented in cooperation by :
 * 
 * Dept. of Agrohydrology             Dept. of Water Resources
 * Winand Staring Centre              Wageningen Agricultura
 *                                    University
 * Marijkeweg 11/22                   Nieuwe Kanaal 11
 * 6700 AC  Wageningen                6709 PA  Wageningen
 * The Netherlands                    The Netherlands
 * Fax: +31 8370 24812                Fax: +31 8370 84885
 */

static char RCSid[] =
 "$Id: vamps.c,v 0.40 1999/01/06 12:13:01 schj Alpha $";

#define TRY_TOPSYS

#include "nrutil.h"	/* nrutil type matrix operations */
#include "getopt.h"
#include "swatsoil.h"
#include "py_init.h"
#include "deffile.h"	/* input/output ini-file library */
#include "vamps.h"
#define VAMPS
#ifndef NOMAQ
#include "marquard.h"
#endif
#ifdef TRY_TOPSYS
#include "../topsys/topsys.h"
#endif 

#include <math.h>
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <strings.h>

/* cpusec: CPU time elapsed in seconds (was in init_s.c) */
static int ft = 0;
static int bclk = 0;
double cpusec()
{
	if (ft == 0){
		bclk = (int)clock();
		ft = 1;
	}
#ifdef CLOCKS_PER_SEC
#undef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif
#ifdef CLK_TCK
	return (double)((int)clock() - bclk)/(double)CLK_TCK;
#else
	return -1.0;
#endif
}

/* Command line options...*/
#define OPTSTR "fFE:L:vnclsho:O:HC:D:S:I:P:"

#define USAGE "\
\n\t[-v][-h][-c][-f][-l][-O setname][-H][-C commentchars]\
\n\t[-D variable][-s][-S section name value][-o filename]\
\n\t[-n][-E speed][-L file] filename"
#define EXPL "\
\t--verbose\t\tbe verbose about performed actions\n\
\t--version\t\tshow version of vamps\n\
\t--help\t\t\tshow this information\n\
\t--Logfile file\t\tlog session to file\n\
\t--copyright\t\tshow part of the GPL\n\
\t--Output setname\tdump specified setname\n\
\t--Determine variable\tdetermine the variable specified\n\
\t--output filename\tsend output to filename\n\
\t--showdef\t\tsend defaults to stdout\n\
\t--noinputdump\t\tdon't include input file in output\n\
\t--spEed speed\t\tset calculation speed to speed (1-6)\n\
\t--Comment char\t\tset char(s) as commentchars\n\
\t--Setvar section name value\tOverride value from input file\n\
"

/*
* GNU long options 
*/
struct option options[] =
{
	{"help", no_argument, 0, (int)'h'},
	{"copyright", no_argument, 0, (int)'c'},
	{"verbose", no_argument, 0, (int)'v'},
	{"version", no_argument, 0, (int)'V'},
	{"license", no_argument, 0, (int)'l'},
	{"showdef", no_argument, 0, (int)'s'},
	{"inputdump", no_argument, 0, (int)'i'},
	{"output", required_argument, 0, (int)'o'},
	{"Logfile", required_argument, 0, (int)'L'},
	{"Setvar", required_argument, 0, (int)'E'},
	{"speed", required_argument, 0, (int)'S'},
	{"Output", required_argument, 0, (int)'O'},
	{"Header", no_argument, 0, (int)'H'},
	{"Comment", required_argument, 0, (int)'C'},
	{"Determine", required_argument, 0, (int)'D'},
#ifdef HAVE_LIBPYTHON
	{"Python", required_argument, 0, (int)'P'},
#endif
	{0, 0, 0, 0},
};


int max_sec_in_ts = MAX_SEC_IN_TS;
int sl_in_input = 0;   /* legacy: unused */
int 	vers_char = VERS_CHAR;
int     version = VERSIONNR;
char    infilename[1024];
char    outfilename[1024];
char	xtraoutfn[1024];
char    vampslib[1024];
char    rundir[512]=".";
char	runsuffix[512]="NO SUFFIX";
int     usesuffix = FALSE;
int     iniinmem = TRUE;
int     doxtraout = FALSE;
int 	exit_is_inter = 0;
#ifdef VAMPS_EXT_BUILD
/* plot.c is not linked in the extension build; provide a stub definition. */
char graphcommand[256] = "";
#endif

int VampsPid = 0;
char   *progname;

#ifdef HAVE_LIBPYTHON
static char py_script_path[1024] = "";
static int  have_py_script = 0;
#endif

int     days = 0;
double  starttime;             /* Start-time as specified by the user, 
				or determined from endpos */
double  endtime;               /* end-time as specified by the user or 
				determined from steps and startpos */
int     startpos = 0;          /* Start position in precipitation array */
int     steps = 0;
int     thisstep = 0;
extern  char *progstr;
FILE    *genoutfile = NULL;    /* general output of run */
FILE    *xoutfile = NULL;      /* xtra time-series output */
double  thiststep = 1.0;       /* actual length of this timestep in days. 
				For the first step 1.0 is assumed 
				(from initial to firstentry in the precip 
				 file)*/

void loaddefaults (char *defname);
void showusage (char *argv[], int verb);
void showdef (char *defname);
extern void outfileheader (char *name);



FILE   *infile;
char    s[1024];
char *cmline;
int     dumpini = TRUE;
int     Dsoil, Dcanop;

extern void exitfunc(void);

/*C:main
 *@int main (int argc, char *argv[])
 *
 * The main vamps program 
 */
#ifndef VAMPS_EXT_BUILD
int
main (int argc, char *argv[])
{
	int  what;

	atexit(&exitfunc); /* clean stuff at exit */

	VampsPid = getpid();
	(void)cpusec(); /* initialize counter */
	if (argc <=1 && strcmp(argv[0],"ivamps") != 0) {
		showusage (argv, 0);
		exit(EXIT_FAILURE);
	}

	setsig (); /* sets up signal handling, kind of broken with DOS */
	
	if (getenv("VAMPSLIB")) /* Try and find library files..*/
		strcpy(vampslib,getenv("VAMPSLIB"));
	else
		strcpy(vampslib,VAMPSLIB);

	strcpy (infilename, "nothing yet");
	strcpy (outfilename, "nothing yet");
	strcpy (logfilename, "vamps.log");

	progstr = (char *) ts_memory (NULL, 3 * sizeof (char), progname);
	strcpy (progstr, "2");
	progname = argv[0];
	strcpy(deffname,getdefname (DEFNAME));
	loaddefaults (deffname);/* loads defaults from deffname  */

	cmline= ts_command (argc, argv);
	/* Parse the comand line and set options */
	while ((what = getopt_long(argc, argv, OPTSTR, options, NULL)) != EOF){
		switch (what){
			case 'h':
				showusage (argv, 1);
				exit (0);
				/* NOTREACHED */
			case '?':
				showit("vamps",ERR,"Option not known",0,0);
				showusage (argv, 1);
				exit (EXIT_FAILURE);
				/* NOTREACHED */
			case 'E':
				sprintf(s,"soil speed %d",atoi(optarg));
				setvar (s);
				break;
			case 'S':
				setvar (optarg);
				break;
#ifdef HAVE_LIBPYTHON
			case 'P':
				strncpy(py_script_path, optarg, 1023);
				py_script_path[1023] = '\0';
				have_py_script = 1;
				break;
#endif
			case 'V':
				(void) fprintf(stderr,"%.2f%c\n",(double)version/100.0,vers_char);
				exit(0);
				/* NOTREACHED */
			case 'L':
				loggen = TRUE;
				strcpy (logfilename, optarg);
				break;
			case 'v':
				verbose++;
				defverb = verbose;
				break;
			case 'c':
				disclaim (argv[0]);
				exit (0);
			case 'O':
				(void)to_outlist (optarg);
				break;
			case 'D':
				(void) sprintf (s, "determine %s 1", optarg);
				setvar (s);		/* Put into override list */
				break;
			case 'C':
				strcpy(commchar, optarg);
				break;
			case 'H':
				header = FALSE;
				break;
			case 'n':
				dumpini = FALSE;
				break;
			case 'l':
				disclaim (argv[0]);
				exit (0);
			case 'o':
				strcpy(outfilename,optarg);
				break;
			case 's':
				showdef (deffname);
				exit (0);
			default:
				break;
		}
	}

	if (argc > 1)
		strcpy (infilename, argv[argc - 1]);

	strcpy (infilename, argv[argc - 1]);

	prelim();
	dorun();
	dotail();

	/* deletes the ini memory list (if present)  */
	
	return 0;
}

/*C:dotail
 *@ Void dotail()
 * 
 * Does 'tail' part of a model run. Cleans op files and memory. Also saves
 * some stuff to disk.
 */ 
void
dotail()
{
	int i;
	extern void settotzero();
	
	/* Clean up stuff */
	if (Dsoil)
		postsoil ();
#ifdef TRY_TOPSYS
#else	
	if (Dcanop)
		postcanop ();
#endif	
	closeout ();
	xcloseout ();
	/* Scan the outlist array and dump the sets that are found */
	/* for (i = 0; i < outnr; i++)
		dumpset (getsetbyname (outlist[i]), (FILE *) NULL); */


	/* Dump the contents of the input file to the output file
	 * if specified */
	if (dumpini)
		inidump();

	endshow ();

	outfiletrailer ();

	/* Close the output stream if needed  */
	(void)fclose (genoutfile);

#ifdef HAVE_LIBPYTHON
	if (have_py_script) {
		py_at_end_f();
		finalize_py();
	}
#endif

	settotzero();
	/*show_tmplist();*/
}
#endif /* VAMPS_EXT_BUILD */

/*C:dorun
 *@ Void dorun()
 * 
 * Does the main body of the model run. From the first to the last
 * timestep. 
 */ 
#ifndef VAMPS_EXT_BUILD
void
dorun()
{
	int i;

	/* get length of first timestep (initial to first record )
	 * by default this is set to the interval between step 0
	 * and 1 or is startpos is specified between startpos and
	 * startpos -1*/
	thiststep = getdefdoub ("time", "firststep",data[id.pre].xy[1+startpos].x- data[id.pre].xy[startpos].x, infilename, FALSE);
	t = data[id.pre].xy[startpos].x - thiststep;
	starttime = t;
	endtime = data[id.pre].xy[startpos + steps - 1].x;
	i = startpos;

	/* Start the main loop over the Canopy and soil modules.
	 * Note that both modules are only coupled once every
	 * timestep and _NOT_ in the intermediate itterations. This
	 * should not give much problems if the timestep is small
	 * enough */
	while (/*t >= starttime && */t < endtime){
		alarm(max_sec_in_ts);

		thisstep = i;
		if (i != startpos){
			thiststep = data[id.pre].xy[i].x - data[id.pre].xy[i - 1].x;
			if (thiststep <=0.0)
				Perror(progname,1,0,RCSid,"Timestep <=0","Check P input data");
		}
		utout (&i);
#ifdef TRY_TOPSYS
		tstep_top(i, &data[id.pre].xy[i].y, &data[id.inr].xy[i].y,
				&data[id.ptr].xy[i].y, &data[id.spe].xy[i].y);
#else
		if (Dcanop)
			tstep_canop (&i);
#endif
		if (Dsoil)
			tstep_soil (i,data[id.pre].xy[i].x, data[id.pre].xy[i].y,
					data[id.inr].xy[i].y, data[id.ptr].xy[i].y,
					data[id.spe].xy[i].y);
		if (doxtraout)
			xtraout (i);
#ifdef HAVE_LIBPYTHON
		if (have_py_script)
			py_each_step_f();
#endif
		i++;
		if (!Dsoil) /* we have to increase time*/
			t+=thiststep;
	}
}
#endif /* VAMPS_EXT_BUILD */

/*C:disclaim
 *@ void disclaim(char *progname)
 *
 * shows a short version of the GNU public licence on stderr
 */
void
disclaim (char *progname)
{
	(void) fprintf (stderr, "%s:\n%s\n", progname,GNUL);
}

/*C:showusage
 *@ void showusage( char *argv, int verb)
 *
 * shows vamps program indentification and a short (verb == 0 )
 * or long (verb >0 ) command line explanation.
 */
void
showusage (char *argv[],int verb)
{
	(void) fprintf (stderr, "\n%s - %s\n\t%s (%s) %s\n\t%s (c) - %s\n", argv[0], DESCRIPTION, IDENTIFY, STATUS, PROVERSION, AUTHOR, DATE);
	(void) fprintf (stderr, "\tBuild on %s at %s by %s (%s)\n", WHERE, BUILDDATE, WHO, OS);
	(void) fprintf (stderr, "\t(%s)\n", RCSid);
	(void) fprintf (stderr, "\n\tusage: %s %s\n\n", argv[0], USAGE);
	if (verb)
		(void) fprintf (stderr, "%s", EXPL);
}


/*C:loaddefaults
 *@ void loaddefaults ()
 *
 * loads defaults from the defaults-file. After that the defaults
 * in the vamps section from the input file are loaded
 */

void
loaddefaults (char *defname)
{

	
	verbose = getdefint ("vamps", "verbose", verbose, defname, FALSE);
	/* fmttime = getdefint ("vamps", "fmttime", fmttime, defname, FALSE);
	 * */
	{ const char *_t = getdefstr ("vamps", "commentchars", commchar, defname, FALSE);
	  if (_t != commchar) strncpy(commchar, _t, 511); }
	header = getdefint ("vamps", "header", header, defname, FALSE);
	max_sec_in_ts = getdefint ("vamps", "max_sec_in_ts", max_sec_in_ts, defname, FALSE);
	loggen = getdefint ("vamps", "logging", loggen, defname, FALSE);
	iniinmem = getdefint ("vamps", "iniinmem", iniinmem, defname, FALSE);
	{ const char *_t = getdefstr ("vamps", "logfilename", logfilename, defname, FALSE);
	  if (_t != logfilename) strncpy(logfilename, _t, 1023); }
	{ const char *_t = getdefstr ("vamps", "progstr", progstr, defname, FALSE);
	  if (_t != progstr) strncpy(progstr, _t, 2047); }
#ifndef VAMPS_EXT_BUILD
	{ const char *_t = getdefstr ("vamps", "graphcommand", graphcommand, defname, FALSE);
	  if (_t != graphcommand) strncpy(graphcommand, _t, 255); }
#endif
#ifdef HAVE_LIBPYTHON
	if (!have_py_script) {
		const char *_t = getdefstr ("vamps", "xtrapy", NULL, defname, FALSE);
		if (_t) {
			/* Try relative to the .inp file's directory first */
			char candidate[1024];
			const char *sep = strrchr(defname, DIRSEP);
			if (sep && *_t != DIRSEP) {
				int dlen = (int)(sep - defname) + 1;
				if (dlen < 1020) {
					strncpy(candidate, defname, dlen);
					candidate[dlen] = '\0';
					strncat(candidate, _t, 1023 - dlen);
					if (access(candidate, R_OK) == 0) {
						strncpy(py_script_path, candidate, 1023);
						py_script_path[1023] = '\0';
						have_py_script = 1;
					}
				}
			}
			if (!have_py_script) {
				strncpy(py_script_path, _t, 1023);
				py_script_path[1023] = '\0';
				have_py_script = 1;
			}
		}
	}
#endif
}

/*-
 * void showdefaults ()
 * shows defaults from the compiled in defaults combined with
 * defaults-file (.vampsrc of vamps.rc)
 */
void
showdef (char *defname)
{
	(void) fprintf (stdout, "#%s  defaults\n", progname);
	(void) fprintf (stdout, "[vamps]\n");
	(void) fprintf (stdout, "verbose = %d\n", getdefint ("vamps", "verbose", verbose, defname, FALSE));
	(void) fprintf (stdout, "commentchars = %s\n", getdefstr ("vamps", "commentchars", commchar, defname, FALSE));
	(void) fprintf (stdout, "max_sec_in_ts = %d\n", getdefint ("vamps", "max_sec_in_ts", max_sec_in_ts, defname, FALSE));
	(void) fprintf (stdout, "header = %d\n", getdefint ("vamps", "header", header, defname, FALSE));
	(void) fprintf (stdout, "logging = %d\n", getdefint ("vamps", "logging", loggen, defname, FALSE));
	(void) fprintf (stdout, "logfilename = %s\n", getdefstr ("vamps", "logfilename", logfilename, defname, FALSE));
	(void) fprintf (stdout, "iniinmem = %d\n", getdefint ("vamps", "iniinmem", iniinmem, defname, FALSE));
	(void) fprintf (stdout, "progstr = %s\n", getdefstr ("vamps", "progstr", progstr, defname, FALSE));
#ifndef VAMPS_EXT_BUILD
	fprintf (stdout, "graphcommand = %s\n", getdefstr ("vamps", "graphcommand",graphcommand, defname, FALSE));
#endif
}

/*-
 *  char *getdefname(char *defname)
 *  searches for the full name of the defaults file
 *  that can be opened. First the current dir, than $HOME, then
 *  VAMPSLIB, then $VAMPSLIB and as last resort the dir in which
 *  vamps resides.
 */
char *
getdefname (char *defname)
{
	FILE   *tst;
	static char   s[1024];
	static char   ss[1024];
	char *tmps;

	strcpy(s,defname);
	strcpy(ss,progname);
	if ((tst = fopen (s, "r")) != NULL)
	{
		(void)fclose (tst);
		return s;
	}

	tmps = getenv ("HOME");
	if (tmps){
		sprintf (s, "%s%c%s", tmps,DIRSEP, defname);
		if ((tst = fopen (s, "r")) != NULL)
		{
			(void)fclose (tst);
			return s;
		}
	}

	sprintf (s, "%s%c%s", VAMPSLIB,DIRSEP, defname);
	if ((tst = fopen (s, "r")) != NULL)
	{
		(void)fclose (tst);
		return s;
	}

	/* Try to get name from program name, note the go32 uses UNIX style slashes*/
	tmps= rindex(ss,DIRSEP);
	if (tmps){
		*tmps = '\0';
		sprintf (s, "%s%c%s", ss,DIRSEP, defname);
		if ((tst = fopen (s, "r")) != NULL)
		{
			(void)fclose (tst);
			return s;
		}
	}

	tmps = getenv ("VAMPSLIB");
	if (tmps){
		sprintf (s, "%s%c%s", tmps ,DIRSEP, defname);
		if ((tst = fopen (s, "r")) != NULL)
		{
			(void)fclose (tst);
			return s;
		}
	}

  return defname;
}





/*C:prelim
 *@ void prelim(void)
 *
 */
void
prelim ()
{
	int i;
	
	/* Do preliminary stuff, get defaults from files */
	loaddefaults (infilename);/* loads defaults from infilename*/

#ifndef VAMPS_EXT_BUILD
	if ((infile = fopen (infilename, "r")) == NULL)
		Perror (progname, 1,1, RCSid, "File open failed:", infilename);
#endif

	{ const char *_t = getdefstr("run","rundir",rundir,infilename,FALSE);
	  if (_t != rundir) strncpy(rundir, _t, 511); }
	/* First command line outputfile, then run section, then stdout*/
	if(strcmp(outfilename,"nothing yet") != 0){
		if ((genoutfile = fopen (outfilename, "w")) == NULL){
			Perror (progname, 1,1, RCSid, "File open failed:", outfilename);
		}
	}else{
		{ const char *_t = getdefstr("run","outputfile",outfilename,infilename,FALSE);
		  if (_t != outfilename) strncpy(outfilename, _t, 1023); }
		if(strcmp(outfilename,"nothing yet") != 0){
			if ((genoutfile = fopen (outfilename, "w")) == NULL){
				Perror (progname, 1,1, RCSid, "File open failed:", outfilename);
			}
		}else    
			genoutfile = stdout;
	}


	/*
	 * Start the outfile generation and print a small header 
	 */
	outfileheader (infilename);
	printstr("command",cmline);
	printstr("defaultsfile",deffname);

	startshow ("vamps", infilename);
	showit ("vamps",MESG,getdefstr ("environment", "caseid", "None", infilename, FALSE),1,verbose);
	showit ("vamps",MESG,cmline,1,verbose);

	/*
	 * dumpini can only dump the memory list, so we have to read that list 
	 */
	if (dumpini && !iniinmem)
		dumpini = FALSE;

	/*
	 * read the input file into the memory list 
	 */
	if (iniinmem)
	{
		showit ("vamps",MESG,"reading file in memory...",1,verbose);
		rinmem (infilename);
		showit ("vamps",MESG,"done reading ini file in memory",2,verbose);
	}


	days = getdefint ("time", "steps", 0, infilename, TRUE);
	steps = days;
	starttime = getdefdoub ("time", "start", 0.0, infilename, FALSE);
	endtime = getdefdoub ("time", "end", 0.0, infilename, FALSE);
	if (starttime == 0.0)
		startpos = getdefint ("time","startpos",0,infilename,FALSE);
	else
		Perror(progname,1,0,RCSid,"Starttime not yet implemented","Use startpos instead");

	sprintf (s, "run will contain %d steps", steps);
	/* Check for precipitation. In the new setup a precipitation
	 * file _MUST_ be present to derive the timesteps! */
	if ((id.pre = getsetbyname ("pre")) < 0)
	{
		strcpy (s, getdefstr ("ts", "pre", "nothing", infilename, FALSE));
		if (strcmp (s, "nothing") == 0)
			Perror (progname, 1,0, RCSid, "Need precipitation data", "");
		get_data (s, "pre",steps);
		if (data[0].xy == NULL)
			exit(1);
		id.pre = getsetbyname ("pre");
	}

	showit ("vamps",MESG,s,1,verbose);

	/* check for parameters to be determined THIS SHOULD CHANGE!! */
	/*if (getdefint ("determine", "pevaporation", 0, infilename, FALSE)) 
		(void) DoEvap (getdefint ("pevaporation", "method", 1, infilename, FALSE)); */

	/* check for parameters to be determined */
	/*if (getdefint ("determine", "evaporation", 0, infilename, FALSE))
		(void) DoActEvap (getdefint ("evaporation", "method", 1, infilename, FALSE)); */
	/* check for parameters to be determined */

	/* Start soil moisture stuff if needed */
	Dsoil = getdefint ("determine", "soilmoisture", 0, infilename, FALSE);
	Dcanop = getdefint ("determine", "canopy", 0, infilename, FALSE);

	/* Do preliminaries for all to be determined */
#ifndef VAMPS_EXT_BUILD
	openout ();
#endif
#ifndef VAMPS_EXT_BUILD
	strcpy (xtraoutfn, getdefstr ("xout", "filename", "nothing", infilename, FALSE));
	if (strcmp (xtraoutfn, "nothing") != 0){
		sprintf (s, "dumping xtra output to: %s\n",xtraoutfn);
		showit("vamps",MESG,s,1,verbose);
		doxtraout = TRUE;
		xoutfile = fopen (xtraoutfn, "w");
		xopenout ();
	}
#endif
	/* This used to be in dorun */
#ifdef TRY_TOPSYS		
	init_top(getdefint("top","system",TOP_PRE_CANOP,infilename,TRUE)); 
	pre_top(); 
#else
	if (Dcanop) 
		precanop ();
#endif
#ifdef HAVE_LIBPYTHON
	/* Init Python before presoil() so soil method=5 callbacks are available */
	if (have_py_script)
		init_py(py_script_path);
#endif
	if (Dsoil)  presoil ();
#ifdef HAVE_LIBPYTHON
	if (have_py_script)
		py_at_start_f();
#endif

}

void
inidump(void)
{
	printcom(" --> start of input file dump <--\n");
	printcom(" Settings at end of run (ini file dump)\n");
	writememini (NULL, infilename, genoutfile);
	printcom(" --> end of input file dump <--");
}

