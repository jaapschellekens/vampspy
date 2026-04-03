/* $Header: /home/schj/src/vamps_0.99g/src/deffile.lib/RCS/index.c,v 1.9 1999/01/06 12:13:01 schj Alpha $ 
*/

/*  $RCSfile: index.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

/*F:index.c
 *
 * This file contains functions to maintain a list of pointers
 * to sections in a (large) vamps output file. It is only usefull
 * if you process large files that need to be accessed in a more
 * or less random way.
 *
 * An index can be created using @makeindex@ saved to a file with
 * @saveindex@ and read from a file with @readindex@.
 * If an index is present the @pro_getdefault@ function will try
 * and use this. At present an index can only be in memory for one
 * file at a time.
 * */

static char RCSid[] =
"$Id: index.c,v 1.9 1999/01/06 12:13:01 schj Alpha $";

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "deffile.h"

extern void *defmem (void *ptr, size_t size, char *prog);
sectionlistt *sectionlist=NULL; /* list with pointers to sections */
int nrsectionlist=0;/* number of items in section list */
int usesecptr = 1;
int firstpass=1;

/*
typedef struct {
	char *fname;
	sectionlistt  *sectionlist;
}findext;
*/

/*C:getsecpos
 *@ long int getsecpos(const char *section)
 *
 * Searches for the start of @section@ in the current
 * indexlist. Use only in @pro_getdefault@
 *
 * Returns offset that can be passed to @fseek@ or -1 if the section
 * is not found */
long int
getsecpos(const char *section)
{
	int i;

	for (i=0;i<nrsectionlist;i++)
		if (Strcasecmp(sectionlist[i].section,section) == 0){
			return sectionlist[i].secptr;
		}

	return -1;
}

/*C:savesecpos
 *@void savesecpos(const char *section, long int nowpos)
 *
 * Adds @section@ with position @nowpos@ to the indexlist and
 * allocates nen memory to the seclist structure
 *
 * Returns: nothing
 *
 * Remarks: To avoid memory fragmentation new mem is allocated in
 * @atime@ blocks */
void
savesecpos(const char *section,long int nowpos)
{
	const int atime = 128;

	if (nrsectionlist == 0 || nrsectionlist % atime == 0){
		sectionlist = 
			(sectionlistt *)defmem((void *)sectionlist, 
					       (((nrsectionlist+atime)/atime)
						*atime) * sizeof(sectionlistt)
					       , defprog);
	}
	strcpy(sectionlist[nrsectionlist].section,section);
	sectionlist[nrsectionlist].secptr=nowpos;
	nrsectionlist++;
}

/*C:saveindex
 *@ int saveindex(char *fname)
 *
 * Saves the current index to a file. This file can be
 * read with @readindex@
 *
 * Returns: 0 on failure, 1 on success */
int 
saveindex(char *fname)
{
	FILE *sfile;
	int i;

	if ((sfile = fopen(fname,"w"))){
		for (i = 0 ; i< nrsectionlist; i++){
			(void)fprintf(sfile,"%s %ld\n",sectionlist[i].section,sectionlist[i].secptr);
		}
	}else{
		(void)fprintf(stderr,"(%s) coudld not open %s\n",RCSid,fname);
		return 0;
	}

(void)fclose(sfile);
return 1;
}

/*C:delseclist
 *@ void delseclist(void)
 *
 * Resets the @nrsectionlist@ counter and frees the
 * sectionlist memory
 *
 * Returns: nothing */
void
delseclist(void)
{
	if (nrsectionlist > 0 && sectionlist){
		free(sectionlist);
		nrsectionlist = 0;
		sectionlist = NULL;
	}
}

/*C:readindex
 *@ int readindex(char *fname)
 *
 * Reads an index (saved with @saveindex@) from a
 * file and constructs a sectionlist
 *
 * Returns: 0 on error, 1 on success */
int 
readindex(char *fname)
{
	FILE *sfile;
	sectionlistt tmpsec;
	const int atime=128;

	if ((sfile = fopen(fname,"r"))){
		delseclist();
		while (fscanf(sfile,"%s %ld",tmpsec.section,&tmpsec.secptr)
				!= EOF){
			if (nrsectionlist == 0 || nrsectionlist % atime == 0){
				sectionlist = (sectionlistt *)
					defmem((void *)sectionlist,
							(((nrsectionlist+atime)
							  /atime)*atime)
							*sizeof(sectionlistt),
							defprog);
			}
			strcpy(sectionlist[nrsectionlist].section,
					tmpsec.section);
			sectionlist[nrsectionlist].secptr = tmpsec.secptr;
			nrsectionlist++;
		}
	}else{
		if (defverb)
			(void)fprintf(stderr,"(%s) could not open %s\n",
				      RCSid,fname);
		return 0;
	}

	(void)fclose(sfile);
	return 1;
}


/*C:makeindex
 *@ int makeindex(char *fname)
 *
 * Description: Creates a section index from the file fname
 *
 * Returns: 0 on error, 1 on success
 *
 * Remarks:The index can be saved with \myref{saveindex} */
int 
makeindex(char *fname)
{
	FILE *thef;
	char *cp,*rp;
	long int lastpos;

	/* first clear old index */
	delseclist();
	if (!(thef = fopen(fname,"r")))
		return 0;

	/* while ((cp = Fgets (buf, LBUFF, thef)) != (char *) NULL){ */
	while ((cp = fgetl (thef)) != (char *) NULL){
		while (isspace (*cp))
			cp++;
		if (*cp && !strchr(commchar,*cp)){
			if (*cp == '[' && (rp = strchr (cp, ']')) != (char *) NULL) {
				*rp = '\0';
				cp++;
				/* save position information */
				/* We have to account for ^M   */
#if defined(msdos) || defined(os2) || defined(__MSDOS__) || defined(__FAT__) || defined(__GO32__)
				lastpos = ftell (thef) - strlen (cp) - 4;
#else
				lastpos = ftell (thef) - strlen (cp) - 3;
#endif
				savesecpos(cp,lastpos);
				if (defverb)
					(void)fprintf(stderr,"Indexing: %s\r",cp);
			}
		}
	}

	(void)fclose(thef);

	if (defverb)
		(void)fprintf(stderr,"\nDone indexing.\n");

	return 1;
}
