/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/output.c,v 1.17 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: output.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */
#include "vamps.h"
#include <string.h>

#ifdef DDEBUG
static char RCSid[] =
"$Id: output.c,v 1.17 1999/01/06 12:13:01 schj Alpha $";
#endif

/* This should change*/
char outlist[64][128];
int detnr;
int outnr=0;

int
to_outlist(char *s)
{
  outnr++;
  strcpy(outlist[outnr-1],s);

  return 0;
}

/*-
 * inoutlist - returns TRUE if asked set in outlist
 *
 * int inoutlist(char *s)
 *	returns TRUE if S is in outlist
 */
int 
inoutlist(char *s)
{
int	i;
for (i=0;i<outnr;i++)
    if (strcmp(outlist[i],s)==0)
    	return TRUE;
    	
return FALSE;
}
