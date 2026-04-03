/* $Header: /home/schj/src/vamps_0.99g/src/deffile.lib/RCS/strcmp.c,v 1.11 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: strcmp.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#include <string.h>
#include <ctype.h>
#include "deffile.h"

static char RCSid[] =
"$Id: strcmp.c,v 1.11 1999/01/06 12:13:01 schj Alpha $";

/*C:Strcasecmp
 *@ int Strcasecmp(const char *s, const char *ss)
 *
 * Description: compares s and ss ignoring case
 *
 * Returns: 0 if identical, or the position of the first non
 * matching character if not */
int
Strcasecmp (const char *s, const char *ss)
{
	int retval=0;

	while (*s != '\0' && *ss != '\0'){
		if (toupper(*s) != toupper(*ss)){
			retval++;
			break;
		}
		s++;
		ss++;
	}
  
	/* Length difference */
	if ((*s != '\0' || *ss != '\0') && (*s != *ss))
		retval++;

	return retval--;
}
