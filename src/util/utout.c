/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/utout.c,v 1.12 1999/01/06 12:13:01 schj Alpha $ */
/* $RCSfile: utout.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */


static char RCSid[] =
"$Id: utout.c,v 1.12 1999/01/06 12:13:01 schj Alpha $";

#include "vamps.h"
#include "swatsoil.h"
#include "deffile.h"
#define  GENOUT "%.6f "

extern double volini;

/*C:utout 
 *@void utout(int *tstep)
 *
 * Initialize ini type output per timestep
 */
void
utout (int *tstep)
{
  (void)fprintf (genoutfile, "[t_%d]\n", *tstep);
  printfl ("t", data[id.pre].xy[*tstep].x);
  printfl ("tstep", thiststep);
  printfl ("precipitation", data[id.pre].xy[*tstep].y);
  fprintf(genoutfile,"ftoph = %d\n",ftoph);
}

/*C:printfl 
 *@void printfl(const char *des,double fl)
 *
 * Prints the double variable @fl@ with description @des@ 
 * to the output file
 */
void
printfl (const char *des,double fl)
{
  (void)fprintf (genoutfile, "%s=", des);
  (void)fprintf (genoutfile, GENOUT, fl);
  (void)fprintf (genoutfile, "\n");
}

/*C:printint 
 * void printint(const char *des,int intje)
 *
 * Prints the integer variable @intje@ with description @des@ 
 * to the output file
 */
void
printint (const char *des,int intje)
{
  (void)fprintf (genoutfile, "%s=%d\n", des, intje);
}

/*C:printcom 
 *@void printcom (const char *des)
 *
 * prints the comment string @des@ to the output file
 */
void
printcom(const char *des)
{
  (void)fprintf(genoutfile,"%c%s\n",commchar[0],des);
}

/*C:printstr
 *@void printstr(const char *des,const char *str)
 *
 * prints the string @str@ with description @des@ to the
 * output file
 * */
void
printstr(const char *des,const char *str)
{
  (void)fprintf(genoutfile,"%s=%s\n",des,str);
}

/*C:printar 
 *@void printar(const char *des,double *ar, int pts)
 *
 * prints the double array @ar@ containing @pts@ points with
 * description @des@ to the output file
 */
void
printar (const char *des,double *ar,int pts)
{
  int     i;

  (void)fprintf (genoutfile, "%s=", des);
  for (i = 0; i < pts; i++)
    {
      if (!fprintf (genoutfile, GENOUT, ar[i]))
	Perror (progname,1,1, RCSid, "Could not write to genoutfile", "printar()");
      if (i % 6 == 0 && i < pts - 1 && i != 0)
	if (!fprintf (genoutfile, "\\\n"))
	  Perror (progname,1,1, RCSid, "Could not write to genoutfile", "printar()");
    }
  if (!fprintf (genoutfile, "\n"))
    Perror (progname,1,1, RCSid, "Could not write to genoutfile", "printar()");
}

/*- 
 * void openout()
 *
 */
void
openout ()
{
  if (!genoutfile)
    genoutfile = stdout;

  printstr ("infilename", infilename);
  (void)fprintf (genoutfile, "\n[initial]\n");
  printcom("number of steps in modelling");
  printint ("steps", steps);
}

/*-
 * void closeout()
 *
 */
void
closeout ()
{
  if (!fprintf (genoutfile, "\n[final]\n"))
    Perror (progname,1,1, RCSid, "Could not write to genoutfile", "closeout()");
  printcom("Summary of water balance:");
  printfl("precipitation",cumprec);
  printfl("transpiration",cumtra);
  printfl("rootextract",rootextract);
  printfl("interception",cumintc);
  printfl("est-drainage",cumbot);
  printfl("drainage",cqbot);
  printfl("cumtop",cumtop);
  printfl("lateral_drainage",cumdra);
  printfl("soilevaporation",cumeva);
  printfl("initial_storage",volini);
  printfl("final_storage",volact);
  printfl("delta_storage",volini-volact);
  printfl("surface_runoff",surface_runoff);
  printfl("mass_balance",masbal);
  printfl("timestep_calls",(double)nrcalls);
  printfl("nr_it",(double)nr_itter);
  printfl("nr_tri",(double)nr_tri);
  printfl("nr_band",(double)nr_band);
  printfl("nr_hitt",(double)nr_hitt);
  printfl("nr_sat",(double)nr_sat);
  printfl("rtime",(double)(endtime-starttime));
  fflush(genoutfile);
}

void
printsum(FILE *thef)
{
  fprintf(thef,"Number of calls to timestep: %ld\n",nrcalls);
  fprintf(thef,"Number of itterations in headcalc: %ld\n",nr_itter);
  fprintf(thef,"+---------------------------------------------------------------------+\n");
  fprintf(thef,"Summary:\n");
  fprintf(thef,"timesteps\t\t\t%d\n",steps);
  fprintf(thef,"precipitation\t\t\t%g\n",cumprec);
  fprintf(thef,"potential_transpiration\t\t%g\n",cumtra);
  fprintf(thef,"rootextract\t\t\t%g\n",rootextract);
  fprintf(thef,"interception\t\t\t%g\n",cumintc);
  /*fprintf(thef,"drainage (cumbot)\t\t%g\n",cumbot);*/
  fprintf(thef,"drainage (cumbot)\t\t%g\n",cumbot);
  fprintf(thef,"flow through top (cumtop)\t%g\n",cumtop);
  fprintf(thef,"lateral_drainage\t\t%g\n",cumdra);
  fprintf(thef,"soilevaporation\t\t\t%g\n",cumeva);
  fprintf(thef,"initial_storage\t\t\t%g\n",volini);
  fprintf(thef,"final_storage\t\t\t%g\n",volact);
  fprintf(thef,"delta_storage\t\t\t%g\n",volini-volact);
  fprintf(thef,"surface_runoff\t\t\t%g\n",surface_runoff);
  fprintf(thef,"mass-balance\t\t\t%g\n",masbal);
  fprintf(thef,"+---------------------------------------------------------------------+\n");
}
