#include <stdio.h>
#include "deffile.h"

char tf1[] = "test1.ini";
char tf2[] = "test2.ini";
char tf3[] = "test3.ini";
char tfo1[] = "test1.out";
char tfo2[] = "test2.out";
char tfo3[] = "test3.out";
main ()
{
	defverb = 3;
	(void)rinmem(tf1);
	(void)rinmem(tf2);
	(void)rinmem(tf3);
	(void)writememini (tfo1, tf1,NULL);
	(void)writememini (tfo2, tf2,NULL);
	(void)writememini (tfo3, tf3,NULL);
	delfile(tf3);
	delfile(tf2);
	delfile(tf1);
	makeindex(tf2);
	saveindex("test2.idx");
	delseclist();
	makeindex(tf1);
	saveindex("test1.idx");
	delseclist();
	makeindex(tf3);
	saveindex("test3.idx");
	delseclist();
}
