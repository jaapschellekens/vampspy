/* $Header: /home/schj/src/vamps_0.99g/src/deffile.lib/RCS/deffile.c,v 1.36 1999/01/06 12:13:01 schj Alpha $ */

/*  $RCSfile: deffile.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

static char RCSid[] =
"$Id: deffile.c,v 1.36 1999/01/06 12:13:01 schj Alpha $";

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>

#include "deffile.h"

#define DEF_OK  0
#define DEF_CONV_WARN 1 /* conversion in getdef* failed, non fatal */
#define DEF_CONV_ERR 2 /* conversion in getdef* failed, non fatal */
#define DEF_NOTFOUND 3 /* var not found that was needed */
#define DEF_INVALID 4 /* variable not in valid var list */

char    valid_fn[512] = "valvar.ini"; /* filename of file with valid
					 section and variable
					 names. This file should be
					 read into memory before
					 processing the other requests */
int     ckvalid = 0; /* check valid list if set to non zero */
int     def_error = DEF_OK; /* deffile lib error var */
char    commchar[512] = "#%"; /* valid comment characters, "#%" is default */
int     defverb = 0;
int     chk_only_mem = 0; /* Check only mem list if this is true */
char    defprog[] = "libdef";




void *defmem (void *ptr, size_t size, char *prog);
char *pro_getdefault (const char *section,const char *name,const char *fname);


static void ini_exit (const char *section,const char *name,const char *fname);

/*C:ini_exit
 *@static void ini_exit(const char *section,const char *name,
 *@			const char *fname)
 *
 * exit with error 3 if not found
 *
 * Returns: nothing
 */
static void
ini_exit (const char *section,const char *name,const char *fname)
{
  fprintf (stderr, "%s: Fatal:\tcould not find\t->%s<-\n", defprog, name);
  fprintf (stderr, "\tin section\t\t->%s<-\n", section);
  fprintf (stderr, "\tin file\t\t\t->%s<-\n", fname);
  def_error = DEF_NOTFOUND;
  deferror (defprog, 3, RCSid, "Var not found or invalid type,", "see above.");
}

/*C:pro_getdefault
 *@ char *pro_getdefault(const char section,const char *name,const char *fname)
 *
 * gets a string from file with fname, used internally. Actually this
 * is the core of this library
 *
 * Returns: pointer to static buffer if found, otherwise NULL */

static long int lastpos = 0;
/* Save last position in file. This speeds op sequential
   reading of large files. This only works if you use
   opendef() and closedef() */

static FILE *deffp = NULL;

/* File pointer used by opendef and closedef.
   This is usefull for sequential reading of 
   large files.  */

static char    buf[LBUFF];
static char    tmp[LBUFF];


char   
*pro_getdefault (const char *section,const char *name,const char *fname)
{
	char *cp, *rp,*def;
	FILE *thef;
	long int linesread = 0;
	int restart = 0;
	long int ttpos;


	/* first check the defmem override list */
	/* This stuff should GO!!!  Is only used in the -S command
	 * line option of vamps */
	sprintf (buf, "%s  %s", section, name);
	if ((cp = getvar (buf)) != NULL){
		if (defverb > 1){
			sprintf (tmp, "ini: (Override) section=%s, name=%s, value=%s", section, name, cp);
			prit (tmp);
		}
		return cp;
	}

	/* First check the override list for global overrides */
	if ((cp = getvarval (DEF_OVR,section, name)) != NULL)
		return cp; /* overrride */

	/* set a value for global defaults if they exist */
	def = getvarval (DEF_DEF,section, name); /* default */

	/* then check the defmem list */
	if ((cp = getvarval (fname,section, name)) != NULL){
		if (defverb > 1){
			sprintf (tmp, "ini: (Memory) file=%s section=%s, name=%s, value=%s", fname, section, name, cp);
			prit (tmp);
		}
		return cp;
	}

	if (!deffp){
		if (strcmp (fname, "-") == 0)
			thef = stdin;
		else if ((thef = fopen (fname, "r")) == NULL)
			return NULL;
	}else
	thef = deffp;

	if (chk_only_mem == 1){
		if (thef != deffp && thef != stdin)
			(void) fclose (thef);
		if (def) /* return the default */
			return def;
		else
			return NULL; /* oops nothing found */
	}

	/* get stuff from section list to speed up the searching in large
	 * files */
	if (nrsectionlist && usesecptr){
		ttpos=getsecpos(section);
		if (ttpos != -1){
			fseek (thef, ttpos, SEEK_SET);
			lastpos = ttpos;
		}
	}

	while (restart <= 1){
	while ((cp = fgetl (thef)) != (char *) NULL){/* get a line from file..*/
		linesread++;
		while (isspace (*cp))
			cp++;
		if (*cp && !strchr(commchar,*cp)){
		if (*cp == '[' && (rp = strchr (cp, ']')) != (char *) NULL){
			*rp = '\0';
			cp++;
			if (Strcasecmp (section, cp) == 0){	
				/* found the section we need */
			if ((thef == deffp || thef == stdin))
				/*
				 * We have to account for ^M 
				 */
#if defined(msdos) || defined(__GO32__) || defined(os2) || defined(__MSDOS__) || defined(__FAT__)
			lastpos = ftell (thef) - strlen (cp) - 4;
#else
			lastpos = ftell (thef) - strlen (cp) - 3;
#endif
			while((cp = fgetl (thef)) != (char *) NULL){
			linesread++;
			/* cp[strlen (cp) - 1] = '\0'; */
			while (isspace (*cp))
				cp++;
			if (*cp == '[') /* next section, abort */
				break;

			if (*cp && !strchr(commchar,*cp)){
				/* go to = sign */
				for (rp = cp; *rp && *rp != '='; rp++)
					;
				/* Skip trailing non space */
				while (isspace (*rp) || *rp == '=') rp--;
				rp++;		
				if (*rp)
					*rp++ = '\0';
				if (Strcasecmp (name, cp) == 0){
					/* YES! var found */
					rp--;
					*rp = ' ';
					cp = rp;
					while (*cp != '=' && *cp != '\0')/*find =*/
						cp++;
					if (*cp != '\0')
						cp++;
					else
						cp = rp;
					while (isspace (*cp))	/* skip whitespace */
						cp++;
					for (rp = cp; *rp == '\n'; rp++);/*go to end of line  */
					for (rp--; !isspace (*rp); rp--);/*go back to last text*/
					if (*rp)
						*rp++ = '\0';
					if (thef != deffp)
						(void) fclose (thef);
					else   fseek (thef, lastpos, SEEK_SET);
					if (defverb > 1)
					{
						sprintf (tmp, "ini: (File) section=%s, name=%s, value=%s", section, name, cp);
						prit (tmp);
					}
					return cp;
				}
			}
			}
			}
		}
		}
	}
	if (thef == deffp || thef == stdin){
		rewind (thef);
		restart++;
	}
	else{
		restart = 2;
	}
	}

	if (thef != deffp && thef != stdin)
		(void) fclose (thef);
	else  fseek (thef, lastpos, SEEK_SET);

	if (defverb > 2){
		sprintf (tmp, "ini: not found: section=%s, name=%s, value=%s", section, name, cp);
		prit (tmp);
	}
	if (def) /* return default is not found */
		return def;
	else
		return NULL;
}

/*C:opendef
 *@int opendef (char *fname)
 *
 * open a file for processing, close with @closedef()@
 *
 * Returns: 0 on error, otherwise 1
 *
 * Remarks: Opendef is used to speed up processing of files that are
 * used in a _sequential_ way. If files must be accessed randomely
 * @rinimem@ or @readindex@ should be used */
int
opendef (char *fname)
{
	if (deffp) /* is already open, close first */
		(void) closedef();

	if (fname){
		if (strcmp (fname, "-") == 0){
			deffp = stdin;
			return 1;
		}
		if ((deffp = fopen (fname, "r")) == NULL)
			return 0;
	}else
	return 0;

  return 1;
}

/*C:closedef
 *@int closedef ()
 *
 * Description: closes a file previously opened with opendef
 * Returns: fclose's result
 */
int
closedef (void)
{
	int ret = 0;

	if (deffp == stdin)
		return 0;

	if (deffp)
		ret = fclose (deffp);

	deffp = NULL;

	return ret;
}

/*C:getdefstr
 *@char *getdefstr(const char *section,const char *name,char *def,
 *@                char *fname, int exitonerror)
 *
 * Description: Gets the variable name from section section in file
 * fname. Gets the variable name from section section in file fname
 *
 * Returns: def if nothing appropiate is found. It returns a pointer
 * to static memory overwritten at each call */
char   *
getdefstr (const char *section,const char *name,char *def, char *fname, int exitonerror)
{
	char   *thestr;

	thestr = pro_getdefault (section, name, fname);

	if (thestr != NULL)
		return thestr;
	else{
		if (exitonerror)
			ini_exit (section, name, fname);
		return def;
	}
}

/*C:isdefitem
 *@int *isdefitem(const char *section,const char *name, char *fname)
 *
 * Determines if the variable @name@ in section @section@ in file
 * @fname@ is present. The @def_error@ variable is set to
 * @DEF_INVALID@ if the asked variable is not in the list otherwise
 * it is set to @DEF_OK@.
 *
 * Return 1 if present and 0 if not */
int
isdefitem (const char *section,const char *name, const char *fname)
{
	if (pro_getdefault (section, name, fname)){
		def_error = DEF_INVALID;
		return 1;
	}else{
		def_error = DEF_OK;
		return 0;
	}
}

/*C:getdefint
 *@int *getdefint(const char *section,const char *name, int
 *@		*def, char *fname, int exitonerror)
 *
 * Description: Gets the variable name from section section in file
 * fname. Supports FALSE and TRUE and NO and YES strings (translated
 * to 0 and 1)
 *
 * Returns: def if nothing appropiate is found */
int
getdefint (const char *section,const char *name, int def, char *fname, int exitonerror)
{
	char   *thestr;
	char   *endptr;
	int     ttt;

	thestr = pro_getdefault (section, name, fname);

	if (thestr == NULL){
		if (exitonerror)
			ini_exit (section, name, fname);
		return def;
	} else {
		ttt = (int) strtol (thestr, &endptr, 10);
		if (Strcasecmp (thestr, "TRUE") == 0)
			return 1;
		else if (Strcasecmp (thestr, "YES") == 0)
			return 1;
		else if (Strcasecmp (thestr, "FALSE") == 0)
			return 0;
		else if (Strcasecmp (thestr, "NO") == 0)
			return 0;
		if (endptr == thestr){
			if (exitonerror){
				deferror (defprog, 0, RCSid, "Integer conversion failed:", thestr);
				ini_exit (section, name, fname);
			} else {
				deferror (defprog, 0, RCSid, "Integer conversion failed(non-fatal):", thestr);
				deferror (defprog, 0, RCSid, section, name);
				return def;
			}
		}
		return ttt;
	}
}

/*C:getdefar
 *@double *getdefar(const char *section,const char *name,
 *@		double *def, char *fname,*int pts, exitonerror)
  *
  * Description: Gets the variable name (array of doubles) from section
  * section in file fname. The variable @pts@ will hold the number of items
  * in the array
  *
  * Returns: @def@ if nothing appropiate is found or a pointer to malloced mem
  * 
  * Remarks:Return value may be passed to @free()@ */
double *
getdefar (const char *section,const char *name, double *def, char *fname,
		int *pts, int exitonerror)
{
	char   *tt;
	char   *ept;
	double *thear;
	int     i = 0,j;

	*pts = 0;
	tt = pro_getdefault (section, name, fname);
	ept = tt;

	if (tt == NULL){
		if (exitonerror)
			ini_exit (section, name, fname);
		return def;
	}else{
		j = strlen(tt) - 1;
		while (isspace(tt[j--]))
				tt[j] = '\0';
		thear = (double *) defmem ((double *) def, 1024 * sizeof (double), defprog);
		do{
			i++;
			if (i >= 1024)
				deferror (defprog, 1, RCSid, "Array to large", "getdefar()");
			tt = ept;
			thear[i - 1] = strtod (tt, &ept);
		}while (ept != tt);

		i--;
		thear = defmem((double *)thear, i * sizeof(double),defprog);
		*pts = i;
		return thear;
	}
}


/*Cgetdefdoub
 * @double *getdefdoub(const char *section,const char *name,
 * @		double *def, char *fname, exitonrerror)
 *
 * Description: Gets the variable @name@ from section @section@ in
 * file @fname@
 *
 * Returns: @def@ if nothing appropiate is found. Otherwise a double
 * is returned */
double
getdefdoub (const char *section,const char *name, double def, char *fname, int exitonerror)
{
	char   *tt;
	double  ttt;
	char   *endptr;

	tt = pro_getdefault (section, name, fname);

	if (tt == NULL){
		if (exitonerror)
			ini_exit (section, name, fname);
		return def;
	}else{
		ttt = strtod (tt, &endptr);
		if (endptr == tt){
			if (exitonerror){
				deferror (defprog, 0, RCSid, "Double conversion failed:", tt);
				ini_exit (section, name, fname);
			} else {
				deferror (defprog, 0, RCSid, "Double conversion failed(non-fatal):", tt);
				return def;
			}
		}
		return ttt;
	}
	/* NOTREACHED */
}


/*C:issection
 *@int issection(char *section, char *fname)
 *
 * Description: Checks for the existance of a section in file fname
 *
 * Returns:  TRUE if found, FALSE if not
 *
 * Remarks: It first checks the override and memory lists */
int
issection (char *section, char *fname)
{
	FILE *thef;
	char *cp,*rp;

	if (getnspos(section,getnfpos(fname,0),0) != -1)
		return TRUE;

	/* check the file */
	if (!deffp) {
		if (strcmp (fname, "-") == 0) {
			thef = stdin;
		} else {
			if ((thef = fopen (fname, "r")) == NULL){
				return FALSE;
			}
		}
	} else {
		if (deffp)
			thef = deffp;
		else
			return 0;
	}

	while ((cp = fgetl (thef)) != (char *) NULL){
		while (isspace (*cp))
			cp++;
		if (*cp && !strchr(commchar,*cp)) {
			if (*cp == '[' && (rp = strchr (cp, ']')) != 
					(char *) NULL) {
				*rp = '\0';
				cp++;
				if (Strcasecmp (cp, section) == 0) {
					(void) fclose (thef);
					return TRUE;
				}
			}
		}
	}

	(void) fclose (thef);
	return FALSE;
}


/*C:defmem
 *@void *defmem(void *ptr, size_t size, char *defprog)
 *
 * Description: Allocates @size@ bytes of @defmem@ if @ptr == NULL@,
 * else reallocates @size@ bytes from @ptr@ (@ANSI realloc()@). On
 * defmem error, print diagnostic with @defprog@ and exit with status
 * 1
 * Returns: pointer to allocated defmem, deallocate with @free(3)@*/

void *
defmem (void *ptr, size_t size, char *prog)
{
	void   *rp;

	if (ptr)
		rp = (void *) realloc (ptr, size);
	else
		rp = (void *) malloc (size);
	if (rp)
		return (rp);

	(void) fprintf (stderr, "(%s)%s: Memory allocation failed\n", prog, RCSid);
	exit (1);
	/* NOTREACHED */
}



/*C:deferror
 *@void deferror(char *defprog,int exitval, char *from,char
 *@		*descr,char *xtr)
 *
 *Description: Prints an error message on stderr and exits with level
 *exitval if this value is > 0. Normally called with something like:
 *deferror(defprog,1,RCSid,"A fatal error","devision by zero");
 *
 *Returns: nothing
*/
void 
deferror (char *prog, int exitval, char *from, const char *descr,const char *xtr)
{
	if (!exitval){
		(void) fprintf (stderr, "%s:\terror message:\n", prog);
		(void) fprintf (stderr, "\tfrom: %s\n", from);
		(void) fprintf (stderr, "\tdescription: %s %s\n", descr, xtr);
	}else  {
		(void) fprintf (stderr, "%s:\terror message:\n", prog);
		(void) fprintf (stderr, "\tfrom: %s\n", from);
		(void) fprintf (stderr, "\tdescription: %s %s\n", descr, xtr);

		exit (exitval);
	}
}
