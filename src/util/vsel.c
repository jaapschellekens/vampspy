/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/vsel.c,v 1.30 1999/01/06 12:13:01 schj Alpha $ 
 */
/*
 *  $RCSfile: vsel.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

static char RCSid[] =
"$Id: vsel.c,v 1.30 1999/01/06 12:13:01 schj Alpha $";

#define VSEL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vamps.h"
#include "deffile.h"
#include "getopt.h"

void Verror (char *Eprogname, int exitval, int prr, char *from, const char *descr, const char *xtr);

#define VSELDES "extract data from vamps output files"
#define MAXPAR 50
#define COLSAP "\t"

struct option options[] =
{
  {"help", no_argument, 0, 'h'},
  {"copyright", no_argument, 0, 'c'},
  {"index", required_argument, 0, 'i'},     
  {"Index", required_argument, 0, 'I'},     
  {"verbose", no_argument, 0, 'v'},
  {"license", no_argument, 0, 'l'},
  {"memuse", no_argument, 0, 'm'},
  {"starttime", required_argument, 0, 's'},
  {"parameter", required_argument, 0, 'p'},
  {"outfile", required_argument, 0, 'o'},
  {"number", required_argument, 0, 'n'},
  {"endtime", required_argument, 0, 'e'},
  {"time", required_argument, 0, 't'},
  {"Header", no_argument, 0, 'H'},
  {"Comment", required_argument, 0, 'C'},
  {"sKip", required_argument, 0, 'K'},
  {0, 0, 0, 0},
};

#define USE "\t[-h][-v][-H][-c][-l][-m][-K][-o outfile][[-p parameter]...]\
\n\t[[-n layer number]...][[-t time]...][-i filename][-I filename] filename"

#define EXPL "\
\t--verbose\t\tbe verbose about performed actions\n\
\t--help\t\t\tshow this information\n\
\t--copyright\t\tshow part of the GPL\n\
\t--licence\t\tshow part of the GPL\n\
\t--Header\t\tomit header in output\n\
\t--parameter parameter\toutput this parameter (k,h ...)\n\
\t--memuse\t\tuse more memory (some speedup)\n\
\t--index filename\tuse filename as index\n\
\t--Index filename\tgenerate index in filename\n\
\t--outfile file\t\toutput file (- is stdout)\n\
\t--time time\t\tonly output this time\n\
\t--Comment commentchar\tset the commentchar\n\
\t--sKip n\t\tskip n times\n\
\t--number layer\t\toutput only this layer\n\
"

#define OPTSTR "i:I:K:C:mo:t:lcHhvp:n:V"
char *progname;
char command[1024];
char *indexfname;
int mkindex=0;
FILE *outfile = NULL;
FILE *genoutfile = NULL;
double *z;
char section[1024];
char **var;
double ttime;
char *inname = NULL;
int *layer;
int skipset=0;
int verbose =0;

/* Dummy vars for perror.c */
int interpreter(int verb) {return 0;}
int exit_is_inter = 0;
int switchtoint = 0;

int
main (int argc, char *argv[])
{
  int pars = 0;			/*
				 * number of parameters asked 
				 */
  int header = TRUE;
  int skip = 1;			/*
				 * means no skipping 
				 */
  int memsave = TRUE;
  char outfilename[1024];
  int steps;
  double *time;
  int times = 0;
  int layers = 0;
  int i, k, pts;
  int what=0;
  void disclaim ();
  void showinfo (int verb);
  void printit (int varnr);


  usesecptr=0;
  progname = argv[0];

  /*
   * Allocate memory for parameters 
   */
  var = (char **) ts_memory (NULL, MAXPAR * sizeof (char *), progname);
  layer = (int *) ts_memory (NULL, MAXPAR * sizeof (int), progname);
  time = (double *) ts_memory (NULL, MAXPAR * sizeof (double), progname);

  for (i = 0; i < MAXPAR; i++)
    {
      var[i] = (char *) ts_memory (NULL, 1024 * sizeof (char), progname);

      layer[i] = (-999);
    }

  while ((what = getopt_long (argc, argv, OPTSTR, options, NULL)) != EOF)
    {
      switch (what)
	{
	case 'o':
	  {
	    strcpy (outfilename, optarg);
	    if (strcmp (outfilename, "-") == 0)
	      outfile = stdout;
	    else if ((outfile = fopen (outfilename, "w")) == NULL)
	      {
		Verror (progname, 1,1, RCSid, "Could not open file for writing:", outfilename);
	      }
	    break;
	  }
	case 'K':
	  {
	    skip = atoi (optarg);
	    skip = skip <= 0 ? 1 : skip;
	    skipset++;
	    break;
	  }
	case 'I':
	  {
	    mkindex = 1;
	    indexfname = optarg;
	    break;
	  }
	case 'i':
	  {
	    mkindex = 2;
	    indexfname = optarg;
	    break;
	  }
	case 'C':
	  {
	    commchar[0] = *optarg;
	    break;
	  }
	case 'p':
	  {
	    strcpy (var[pars], optarg);
	    pars++;
	    if (pars == MAXPAR)
	      Verror (progname, 1,0, RCSid, "Maximum number of parameters exceeded", "");
	    break;
	  }
	case 'H':
	  {
	    header = FALSE;
	    break;
	  }
	case 'V':
	  {
	    verbose = 6;
	    break;
	  }
	case 'm':
	  {
	    memsave = memsave == FALSE ? TRUE : FALSE;
	    break;
	  }
	case 'v':
	  {
	    verbose = TRUE;
	    break;
	  }
	case 'l':
	  {
	    disclaim (argv[0]);
	    break;
	  }
	case 'c':
	  {
	    disclaim (argv[0]);
	    break;
	  }
	case 'n':
	  {
	    layer[layers] = atoi (optarg);
	    layers++;
	    break;
	  }
	case 'h':
	  {
	    showinfo (1);
	    break;
	  }
	case 't':
	  {
	    time[times] = atof (optarg);
	    times++;
	    break;
	  }
	default:
	  break;
	}
    }

  strcpy (command, ts_command (argc, argv));

   if (!inname)
     inname = argv[optind];

   if (mkindex == 2)
     {
       usesecptr=1;
       readindex(indexfname);
       firstpass = 0;
     }
   else{
    if(mkindex == 1){
      usesecptr=1;
      makeindex(inname);
      saveindex(indexfname);
      exit(0);
   }
   }
   
   
  if (pars < 1)
    showinfo (0);

   if (!memsave && !mkindex)
    {
      rinmem (inname);
    }
  else
    {
      if (!opendef (inname))
	Verror (progname, 1,1, RCSid, "could not open infile:", inname);
      inname = (char *) NULL;
    }

  steps = getdefint ("initial", "steps", 0, inname, TRUE);
  z = getdefar ("initial", "z", NULL, inname, &pts, FALSE);
#ifdef __MSDOS__
  pts--;
#endif

  if (!layers)
    layers++;


  if (!outfile)
    outfile = stdout;

  if (header)
    fprintf (outfile, "%s\n", command);

  if (!skipset) /* default, check for skipfactor in file */
  	skip=getdefint("initial","outskip",skip,inname,FALSE);

  for (k = 0; k < pars; k++)
    {
      i = 0;
      if (header)
	(void) fprintf (outfile, "%c this is %s\n", commchar[0], var[k]);
      do
	{
	  if (!(i % skip))
	    {
	      sprintf (section, "t_%d", i);
	      ttime = getdefdoub (section, "t", (-50.0), inname, FALSE);
	      /* bail out */
	      if (ttime == -50.0 && k >= pars - 1)
		{
		  if (memsave)
		    (void) closedef ();
		  delmemlist ();
		  exit (0);
		}
	      else if (ttime == time[0] || times == 0)
		printit (k);
	    }
	    i++;
	}
      while (ttime >= 0.0);

      if (outfile != stdout)
	fclose (outfile);
    }

  if (memsave)
    (void) closedef ();

  delmemlist ();		/*
				 * deletes the memory list (if present) 
				 */

  return 0;
}


void
printit (int varnr)
{
  int j, pts = 0;
  double *outar;

  outar = getdefar (section, var[varnr], NULL, inname, &pts, FALSE);
  if (outar)
    {
      if (pts > 1 && layer[0] == -999)
	{
	  for (j = 0; j < pts; j++)
	    fprintf (outfile, "%f%s%f\n", z[j], COLSAP, outar[j]);
	}
      else
	{
	  if (pts != 1)
	    fprintf (outfile, "%f%s%f\n", ttime, COLSAP, outar[layer[0]]);
	  else
	    fprintf (outfile, "%f%s%f\n", ttime, COLSAP, outar[0]);
	}
     free(outar);
    }
}

/*-
 * void disclaim(char *progname)
 *
 *  Shows part of GPL
 */
void
disclaim (char *progname)
{
  (void) fprintf (stderr, "%s:\n%s\n", progname,GNUL);
  exit (0);
}

/*-
 * void showinfo(int verb)
 *
 *  Shows some help info
 *
 */
void
showinfo (int verb)
{
  (void) fprintf (stderr, "\n%s - %s\n\t(c) %s - %s\n", progname, VSELDES, AUTHOR, DATE);
  (void) fprintf (stderr, "\tfor vamps %s\n",PROVERSION);
  (void) fprintf (stderr, "\tBuild on %s at %s by %s (%s)\n", WHERE, BUILDDATE, WHO, OS);
  (void) fprintf (stderr, "\t(%s)\n", RCSid);
  (void) fprintf (stderr, "\n\tusage: %s\n%s\n", progname, USE);
  if (verb)
    (void) fprintf (stderr, "%s", EXPL);
  (void) fprintf (stderr, "\n");
  exit (1);
}

void Verror (char *Eprogname,int exitval,int prr,char *from,const char *descr,const char *xtr)
{
	if (!exitval) {
		(void) fprintf (stderr, "%s:\terror message:\n", Eprogname);
		(void) fprintf (stderr, "\tfrom: %s\n", from);
		(void) fprintf (stderr, "\tdescription: %s %s\n", descr, xtr);
		if (prr)
			perror("syserr");
	} else {
		(void) fprintf (stderr, "%s:\terror message:\n", Eprogname);
		(void) fprintf (stderr, "\tfrom: %s\n", from);
		(void) fprintf (stderr, "\tdescription: %s %s\n", descr, xtr);
		if (prr)
			perror("syserr");
		delmemlist ();
		exit (exitval);
	}
}
