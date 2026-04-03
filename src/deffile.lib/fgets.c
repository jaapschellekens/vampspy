/* $Header: /home/schj/src/vamps_0.99g/src/deffile.lib/RCS/fgets.c,v 1.11 1999/01/06 12:13:01 schj Alpha $ */
/*
 *  $RCSfile: fgets.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 *
 */

#include <stdio.h>
#include <string.h>
#include "deffile.h"

static char RCSid[] =
"$Id: fgets.c,v 1.11 1999/01/06 12:13:01 schj Alpha $";

/* NOTE: Fgets not used anymore, see fgetl! */
/*C:Fgets
 *@char *Fgets(char *s,int size,FILE *inf)
 *
 * Description: Replacement fgets, reads past newlines if escaped with
 * \. It should also dump cr '\r' characters
 *
 * Returns: pointer to *s
 *
 * Remarks: It's behaviour is different from fgets. If the buffer is
 * to small a  warning is printed and the truncated buffer is returned
 */

char *
Fgets (char *s,int size,FILE *inf)
{
	int c, i;

	i = c = 0;

newl:
	while (--size > 0 && (c = fgetc (inf)) != EOF && c != '\n'){
		if (c!='\r')
			s[i++] = c;
	}

	if (size <= 1){
		deferror (defprog,0, RCSid, "Warning Input string to large", "Fgets()");
		return s;
	}

	if (c == '\n' && (s[i - 1] == '\\')){
		s [i-1]=' ';
		goto newl;
	}

	if (c == '\n')
		s[i++] = c;

	s[i] = '\0';

	if (i < 1)
		return (char *) NULL;
	else
		return s;
}



/*C:fgetl
 *@ char *fgetl(FILE *fp)
 *
 *	Get the next line from open file fp up to the newline which
 *	is replaced with NUL. Returns char * to the NUL terminated
 *	string or NULL on EOF or error; return ptr points to static
 *	memory overwritten at each invocation.
 *	The function reads across newline chars if escaped with '\'
 *	(not yet!!!!!)
 *
 *	Function written by R. Venneker, Adapted by J. Schellekens
 */
char *
fgetl(FILE *fp)
{
	int	c ='\0',cc = '\0', n;
	static int	nb = 0;
	static char	*buf;

	if(!nb)
		buf = (char *)malloc((nb = 64) * sizeof(char));
	n = 0;
	while((c = getc(fp)) != EOF) {
		if (c == '\r'){ /* 'handle' returns ..*/
			n--;
			continue;
		}

		if (c == '\n' && cc != '\\')
			break;

		if (c == '\n'){ /* escaped newline */
			n--;
			continue;
		}

		if(n >= nb) {
			buf = (char *)realloc(buf, (nb += 64) * sizeof(char));
		}
		buf[n++] = (char)c;
		cc = c;
	}
	if(!n && c != '\n')
		return((char *)NULL);

	buf[n] = '\0';
	return(buf);
}
