/* $Header: /home/schj/src/vamps_0.99g/src/main/RCS/plot.c,v 1.8 1999/01/06 12:13:01 schj Alpha $ */
/* $RCSfile: plot.c,v $
*  $Author: schj $
*  $Date: 1999/01/06 12:13:01 $
*/

static char RCSid[] = "$Id: plot.c,v 1.8 1999/01/06 12:13:01 schj Alpha $";



#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "vamps.h"
#include "swatsoil.h"
#include "soils.h"
#include "marquard.h"
#include "deffile.h"

char *scan_gp_cmd(char *cmd);
char tmp_prefix[128];
void gp_command(char *gnuplot);
FILE *gp_cmd_file;
extern int VampsPid;

#ifndef GNUPLOT
#define GNUPLOT "/usr/local/bin/gnuplot"
#endif

char graphcommand[256] = GNUPLOT;
/* char tmpfname[256]; */
char *tmpfname;

void gp_command(char *gnuplot)
{
#ifdef HAVE_POPEN
	gp_cmd_file = popen(gnuplot, "w");
#else
	Perror(progname,0,0,RCSid,"popen not present on this system","Check configure");
#endif
}



void
gp_close()
{
#ifdef HAVE_POPEN
	if (gp_cmd_file)
		pclose(gp_cmd_file);
	gp_cmd_file = NULL;
#else
	Perror(progname,0,0,RCSid,"popen not present on this system","Check configure");
#endif
}

