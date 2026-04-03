/* $Header: /home/schj/src/vamps_0.99g/src/maq.lib/RCS/strins.c,v 1.6 1999/01/06 12:13:01 schj Alpha $ */
/* 
 *  $RCSfile: strins.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#include <string.h>
#include "marquard.h"

/*-
 * strrep - replace a substring in a string
 *
 * char *strrep(char *s,char *rep,char *with);
 *
 * the string s is not! changed.
 *
 */

char
*strrep(char *s,char *rep,char *with)
{
char *pt;
char *ps;
static char buff[1024];
char *tmp;
int	i;

tmp=buff;

if ((ps=strstr(s,rep))==NULL)
	return s;

strcpy(tmp,s);
pt=ps;

for(i=0;i<strlen(rep);i++)
	pt++;


while (s!=ps){
	*tmp=*s;
	s++;
	tmp++;
}

while (*with!='\0'){
	*tmp=*with;
	tmp++;
	with++;
}

while (*pt!='\0'){
	*tmp=*pt;
	pt++;
	tmp++;
}
*tmp='\0';

return buff;
}

/*-
 * strins - insert a string into another string at a given position
 *
 * char	*strins(char *s,char *ct,char *pos)
 *
 * string s must be large to hold the extra chars. The original
 * string is changed!
 */

char *
strins (char *s,char *ct,char *pos)
{
  char tmp[1024];
  register char *p = s;


  while (p != pos)
    ++p;

  strcpy (tmp, pos);

  while ((*p++ = (*ct++)))
    ;

  strcat (s, tmp);

  return s;
}

