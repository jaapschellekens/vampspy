/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/getstr.c,v 1.19 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: getstr.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

/*
 * getstr.c - extract var from vamps output file */

static char RCSid[] =
"$Id: getstr.c,v 1.19 1999/01/06 12:13:01 schj Alpha $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vamps.h"
#include "deffile.h"
#include "getopt.h"

#define VSELDES "extract var from vamps output file"
#define MAXPAR 12

struct option options[] =
{
  {"help", no_argument, 0, 'h'},
  {"copyright", no_argument, 0, 'c'},
  {"license", no_argument, 0, 'l'},
  {"Comment", required_argument, 0, 'C'},
  {"section", required_argument, 0, 's'},
  {"index", required_argument, 0, 'i'},
  {"name", required_argument, 0, 'n'},
  {"exit", no_argument, 0, 'e'},
  {"print-as-array", no_argument, 0, 'p'},
  {"vsebose", no_argument, 0, 'v'},
  {0, 0, 0, 0},
};

#define USE "\t[-h][-c][-e][-l][-i indexfile]\n\t\t[[-s section -n name]...] filename"
#define EXPL "\
\t--help\t\t\tshow this information\n\
\t--copyright\t\tshow part of the GPL\n\
\t--licence\t\tshow part of the GPL\n\
\t--Comment commentchar\tset commentchar\n\
\t--exit\t\t\tset only exit status (1 if found)\n\
\t--print-as-array\tTreat variable as an array\n\
\t--section\t\tsection in the file\n\
\t--name\t\t\tthe variable in the section\n\
\t--index indexfile\tuse indexfile as a section index\n\
"
#define OPTSTR "s:lcvhn:C:epi:"
char *progname;
FILE *genoutfile = NULL;

/* Dummy vars for perror.c */
int interpreter(int verb) {return 0;}
int exit_is_inter = 0;
int switchtoint = 0;

int
main (int argc,char *argv[])
{
  int verbose =0;
  char **name;
  char **section;
  char *inname = NULL;
  void disclaim (char *);
  void showinfo (int verb);
  double  *thetar;
  int	prar=FALSE;
  int  pts;
  int what;
  int secs=0;
  int names=0;
  int quiet = FALSE;
  int i;
  int retval=0;

  progname = argv[0];

  if (argv[1] == NULL)
    {
      showinfo (0);
    }

  section = (char **) ts_memory (NULL, MAXPAR * sizeof (char *), progname);
  name = (char **) ts_memory (NULL, MAXPAR * sizeof (char *), progname);

  for (i = 0; i < MAXPAR; i++)
    {
      section[i] = (char *) ts_memory (NULL, 128 * sizeof (char), progname);
      name[i] = (char *) ts_memory (NULL,128 * sizeof (char), progname);
    }

  if (*argv[argc - 1] != '-' || strcmp (argv[argc - 1], "-") == 0)
    {
      inname = argv[argc - 1];
      if (!opendef (inname))
	Perror (progname, 1,1, RCSid, "Could not open:", inname);
    }

  while ((what = getopt_long (argc, argv, OPTSTR, options, NULL)) != EOF)
    {
      switch (what)
	{
	case 'l':
	  {
	    disclaim (argv[0]);
	    break;
	  }
	case 'i':
	  {
	    FILE *ttfile;
	    usesecptr =1;
	    defverb = verbose;

	    if ((ttfile = fopen(optarg,"r")) != NULL){
	      (void)fclose(ttfile);
	      readindex(optarg);
	    }else
	      Perror(progname,1,1,RCSid,"Error opening index:",optarg);
	    break;
	  }

	case 'e':
	  {
	    quiet = TRUE;
	    break;
	  }
	case 'v':
	  {
	    verbose++;
	    break;
	  }
	case 'p':
	  {
	    prar = TRUE;
	    break;
	  }
	case 'C':
	  {
	    commchar[0] = *optarg;
	    break;
	  }
	case 'c':
	  {
	    disclaim (argv[0]);
	    break;
	  }
	case 's':
	  {
	    strcpy (section[secs], optarg);
	    secs++;
	    break;
	  }
	case 'n':
	  {
	    strcpy (name[names], optarg);
	    names++;
	    break;
	  }
	case 'h':
	  {
	    showinfo (1);
	    break;
	  }
	default:
	  break;
	}
    }

  if ((names + secs) < 2)
    showinfo (1);


  if (names != secs)
	showinfo(1);

  for (i=0; i < (names+secs) / 2; i++)
    {
      if (quiet)
	{
	  if(!getdefstr (section[i], name[i], NULL, NULL, FALSE))
	     retval++;
	}
      else{
	if (prar){
	   thetar=getdefar (section[i], name[i], NULL, NULL,&pts, TRUE);
	   for (i=0;i<pts;i++)
		   fprintf(stdout,"%g\n",thetar[i]);
	}else
	fprintf (stdout, "%s\n", getdefstr (section[i], name[i], NULL, NULL, TRUE));
      }
    }

  if (inname)
    closedef ();
  if (quiet){
    if (retval)
      return 0;
    else
      return 1;
  }
  else
    return 0;
}

void
disclaim (char *progname)
{
  (void) fprintf (stderr, "%s:\n%s\n", progname, GNUL);
  exit (0);
}

void
showinfo (verb)
     int verb;
{
  (void) fprintf (stderr, "%s - %s\n\t(c) %s - %s\n\t(%s)", progname, VSELDES, AUTHOR, DATE, RCSid);
  (void) fprintf (stderr, "\n\tfor vamps %s",PROVERSION);
  (void) fprintf (stderr, "\n\tBuild on %s at %s by %s (%s)", WHERE, BUILDDATE, WHO, OS);
  (void) fprintf (stderr, "\n\n\tusage:\n\t%s %s\n\n", progname, USE);
  if (verb)
    (void) fprintf (stderr, "%s", EXPL);
  exit (1);
}
