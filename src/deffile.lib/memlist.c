/* $Header: /home/schj/src/vamps_0.99g/src/deffile.lib/RCS/memlist.c,v 1.9 1999/01/06 12:13:01 schj Alpha $ 
 */
/*  $RCSfile: memlist.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */
/*F:memlist.c
 *
 * Functions use to create and maintain a memory list of a vamps
 * input file. This allows for speedup, editing and saving these
 * files. 
 * 
 * A file with the name DEF_OVR (this is a #define) will serve
 * as an override list. A file with the name DEF_DEF will contain 
 * defaults that are used as a last resort. Usually DEF_DEF = _defaults_
 * and DEF_OVR = _override_
 * The code that handles this is actually in @deffile.c@. 
 * */

static char RCSid[] =
"$Id: memlist.c,v 1.9 1999/01/06 12:13:01 schj Alpha $";

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "deffile.h"

static ftype *fdata = NULL;
static int nrf = -1; /* number of files in memory */

static varst  *vars = (varst *) NULL;
static int     nrvars = -1;

extern void *defmem (void *ptr, size_t size, char *prog);

/*:getnfpos
 *@ int getnfpos(const char *fname, int n)
 *
 * Gets a free position for adding a new mem list of a file. If
 * the list is full a new possition is allocated if the n flag > 0 */
int getnfpos(const char *fname, int n)
{
	int i,npos = -1;

	for (i = 0; i < nrf; i++){
		if (strcmp(fname,fdata[i].fname) == 0){
			npos = i;
			break;
		}
	}
	if (npos == -1 && n > 0){
		if (nrf == -1)
			nrf++;

		npos = nrf;
		nrf++;
		/* Allocate new memory ... */
		fdata = (ftype *) defmem ((void *) fdata,
				nrf * sizeof (ftype), defprog);
		if((fdata[npos].fname = strdup(fname)) == NULL)
			perror(RCSid);
		fdata[npos].slist = NULL;
		fdata[npos].secpt = -1;
	}

	return npos;
}

/*:getnspos
 *@ int getnspos(const char *secname,int filenr, int n)
 *
 * Gets a free position for adding a new mem list of a section. If
 * the section is not already present a new possition is allocated */
int getnspos(const char *secname,int filenr, int n)
{
	int i,npos = -1;

	if (filenr >= nrf || !secname)
		return -1;

	for (i = fdata[filenr].secpt - 1; i >=0; i--){
		if (fdata[filenr].slist){
			if (fdata[filenr].slist[i].vpt == -1){
				npos = i;
				if((fdata[filenr].slist[npos].secname = 
					strdup(secname)) == NULL)
					perror(RCSid);
				fdata[filenr].slist[npos].nlist =
					NULL;
				break;
			}
			if (strcmp(secname,fdata[filenr].slist[i].secname)==0){
				npos = i;
				break;
			}
		}
	}

	if (npos == -1 && n > 0){
		if (fdata[filenr].secpt == -1)
			fdata[filenr].secpt++;

		npos = fdata[filenr].secpt;
		fdata[filenr].secpt++;

		/* Allocate new memory ... */
		fdata[filenr].slist = (stype *) defmem ((void *)
				fdata[filenr].slist, 
				(fdata[filenr].secpt) * 
				sizeof (stype), defprog);
		if((fdata[filenr].slist[npos].secname = 
			strdup(secname)) == NULL)
			perror(RCSid);
		fdata[filenr].slist[npos].vpt =  -1;
		fdata[filenr].slist[npos].nlist =  NULL;
	}

	return npos;
}

int 
getvpos(const char *varn, stype *sec)
{
	int i, npos = -1;

	if (!varn || !sec)
		return -1;

	for (i = sec->vpt - 1; i >= 0; i--){
		if (strcmp(varn,sec->nlist[i].var) == 0){
			npos =  i;
			break;
		}
	}

	return npos;
}

/*C:addvar
 *@ int addvar(const char *fname,const  char *secname,const  char *var,
 *@		const  char *val,int nck)
 *
 * Adds variable @var@ with value @val@ to the memorylist in section
 * @secname@ in file @fname@.
 *
 * If nck > 0 then no check for double entries is made.
 * Addvar returns 0 if an existing position is used and 1 if
 * a new position is allocated.
 * */
int 
addvar(const char *fname,const  char *secname,const  char *var,
		const  char *val,int nck)
{
	int fl = -1;
	int sl = -1;
	int ipos;

	fl = getnfpos(fname,1);
	sl = getnspos(secname,fl,1);
	if (nck || (ipos = getvpos(var,&fdata[fl].slist[sl])) == -1){
		if(fdata[fl].slist[sl].vpt == -1)
			fdata[fl].slist[sl].vpt++;
		ipos = fdata[fl].slist[sl].vpt;

		fdata[fl].slist[sl].vpt++;
		fdata[fl].slist[sl].nlist = (vtype *) defmem ((void *)
				fdata[fl].slist[sl].nlist, 
				(fdata[fl].slist[sl].vpt) * 
				sizeof (vtype), defprog);
		fdata[fl].slist[sl].nlist[ipos].var = NULL;
		fdata[fl].slist[sl].nlist[ipos].val = NULL;
		if((fdata[fl].slist[sl].nlist[ipos].var = strdup(var)) == NULL)
			perror(RCSid);
		if((fdata[fl].slist[sl].nlist[ipos].val = strdup(val)) == NULL)
			perror(RCSid);
		fdata[fl].slist[sl].nlist[ipos].hits = 0;
		return 1;
	}else{
		free(fdata[fl].slist[sl].nlist[ipos].val);
		if((fdata[fl].slist[sl].nlist[ipos].val = 
			strdup(val)) == NULL)
			perror(RCSid);

		return 0;
	}
}

char  
*getvarval(const char *fname,const  char *secname,const  char *var)
{
	int fl,sl,vl;

	if ((fl = getnfpos(fname,0)) == -1)
		return NULL;
	if ((sl = getnspos(secname,fl,0)) == -1)
		return NULL;
	if ((vl = getvpos(var,&fdata[fl].slist[sl])) == -1)
		return NULL;
	else{
		fdata[fl].slist[sl].nlist[vl].hits++;
		return fdata[fl].slist[sl].nlist[vl].val;
	}
}

/*C:writememini
 *@ int writememini (char *fname, char *inifile, FILE * stream)
 * 
 * Writes the memlist if fileid @i@ to file @fname@ or
 * stream @stream@ (depending on which of the two ! NULL
 * If @inifile@ is not in memory or @fname@ could not be opened
 * -1 is returned.
 * */
int
writememini (char *fname, char *inifile, FILE * stream)
{
	int j,k,i;
	FILE *out;

	i = getnfpos(inifile,0);
	if (i <0 || i >= nrf)
		return -1;

	if (stream)	/* Dump to stream if given */
		out = stream;
	else {
		if (strcmp (fname, "-") != 0) {
			if ((out = fopen (fname, "w")) == NULL)
				return -1;
		} else
		out = stdout;
	}

	(void)fprintf(out,"%c Written by: %s\n",
		      commchar[0],RCSid);
	(void)fprintf(out,"%c Original file was: %s\n",
		      commchar[0],fdata[i].fname);
	(void)fprintf(out,"%c Number of sections in this file: %d\n",
		      commchar[0],fdata[i].secpt);
	for (j=0; j < fdata[i].secpt; j++){
		if (fdata[i].slist[j].nlist){
			fprintf(out,"\n%c Number of vars in %s: %d\n",
					commchar[0],fdata[i].slist[j].secname,
					fdata[i].slist[j].vpt);
			(void)fprintf(out,"[%s]\n",fdata[i].slist[j].secname);
			for (k = 0; k < fdata[i].slist[j].vpt; k++){
				if (fdata[i].slist[j].nlist[k].hits > 1)
					fprintf(out,"%c Hits: %d\n",
					commchar[0],fdata[i].slist[j].nlist[k].hits);
				(void)fprintf(out,"%s=%s\n",
					      fdata[i].slist[j].nlist[k].var,
					      fdata[i].slist[j].nlist[k].val);		
			}
		}
	}

	if (!stream && out != stdout)
		fclose(out);

	return 0;
}

/*C:getnextvar
 *@int getnextvar(char *fname, char *section, char *name, char *val,
 *@                int reset)
 *
 * get the next variable from the memlist of @fname@. If @reset@ is
 * non zero the pointer is set to the beginning of the list. If the
 * end of the list is reached or the list is non-existent -1 is
 * returned, otherwise 0.  @section@, @name@ and @val@ will hold the
 * values found. @section@, @name@ and @val@ should be large enough to
 * hold @512@ chars.
 * */

int getnextvar(char *fname, char *section, char *name,
		char *val, int reset)
{
	int i,k,j = 0;
	static int lastpos = 0;
	int thispos = 0;

	i = getnfpos(fname,0);
	if (i <0 || i >= nrf)
		return -1;

	lastpos = reset > 0 ? 0 : lastpos;

	for (j=0; j < fdata[i].secpt; j++){
		if (fdata[i].slist[j].nlist){
			for (k = 0; k < fdata[i].slist[j].vpt; k++){
				thispos++;
				if (thispos > lastpos)
					break;
			}
			if (thispos > lastpos)
				break;
		}
		if (thispos > lastpos)
			break;
	}

	if (thispos > lastpos){
		strncpy(val,fdata[i].slist[j].nlist[k].val,511);
		strncpy(name,fdata[i].slist[j].nlist[k].var,511);
		strncpy(section,fdata[i].slist[j].secname,511);
	} else {
		return -1;
	}

	lastpos = thispos;
	return 0;
}

/*C:printsection 
 *@ int printsection(const char fname, const char secname)
 *
 * Prints section @section@ in file @fname@ (in the mem image)
 * to stdout.
 * */
int printsection(const char *fname, const char *secname)
{
	int fl,sl,i;

	if ((fl = getnfpos(fname,0)) == -1)
		return -1;
	if ((sl = getnspos(secname,fl,0)) == -1)
		return -2;

	printf("%%%s in %s:\n[%s]\n",secname,fname,secname);
	for (i=0; i < fdata[fl].slist[sl].vpt; i++){
		printf("%s=%s\n",fdata[fl].slist[sl].nlist[i].var,fdata[fl].slist[sl].nlist[i].val);
	}

	return 0;
}

/*C:delsection
 *@ void delsection(const char fname, const char secname)
 *
 * Deletes section @section@ in file @fname@ (in the mem image)
 * Memory is freed and the position marked as free.
 * */
void delsection(const char *fname, const char *secname)
{
	int fl,sl,i;

	if ((fl = getnfpos(fname,0)) == -1)
		return;
	if ((sl = getnspos(secname,fl,0)) == -1)
		return;

	for (i=0; i < fdata[fl].slist[sl].vpt; i++){
		free(fdata[fl].slist[sl].nlist[i].var);
		free(fdata[fl].slist[sl].nlist[i].val);
	}
	fdata[fl].slist[sl].vpt = -1;
	free(fdata[fl].slist[sl].nlist);
	free(fdata[fl].slist[sl].secname);
	fdata[fl].slist[sl].nlist = NULL;
	fdata[fl].slist[sl].secname = NULL;
}

int
getfid(const char *fname)
{
	int i;

	for (i=0; i < nrf; i++)
		if (strcmp(fdata[i].fname,fname) == 0)
			return i;
	return -1;
}

int
delfile(const char *fname)
{
	int i,j,k;

	if ((i = getfid(fname)) == -1)
		return -1;

	for (j =0; j < fdata[i].secpt; j++){
		for (k=0; k < fdata[i].slist[j].vpt; k++){
			free(fdata[i].slist[j].nlist[k].var);
			free(fdata[i].slist[j].nlist[k].val);
		}
		fdata[i].slist[j].vpt = -1;
		free(fdata[i].slist[j].nlist);
		free(fdata[i].slist[j].secname);
		fdata[i].slist[j].nlist = NULL;
		fdata[i].slist[j].secname = NULL;
	}
	free(fdata[i].fname);
	free(fdata[i].slist);
	fdata[i].fname = NULL;
	fdata[i].slist = NULL;
	fdata[i].secpt = -1;
	nrf--;

	return i;
}

/*C:delmemlist
 *@void delmemlist (void)
 *
 * Description: Deletes the regular defmem list (read with rinmem)
 * for all files.
 * Returns: nothing
 */
void 
delmemlist (void)
{
	int i,j,k; 

	for (i = nrf - 1; i >= 0 ; i--){
		for (j =0; j < fdata[i].secpt; j++){
			for (k=0; k < fdata[i].slist[j].vpt; k++){
				free(fdata[i].slist[j].nlist[k].var);
				free(fdata[i].slist[j].nlist[k].val);
			}
			fdata[i].slist[j].vpt = -1;
			free(fdata[i].slist[j].nlist);
			free(fdata[i].slist[j].secname);
			fdata[i].slist[j].nlist = NULL;
			fdata[i].slist[j].secname = NULL;
		}
		free(fdata[i].fname);
		free(fdata[i].slist);
		fdata[i].secpt = -1;
	}

	free(fdata);
	nrf = -1;
	fdata = NULL;
}

/*C:setvar
 *@int setvar(char *s)
 * 
 * Description: add a value, section name to the mem copy of input file
 * setvar is used to maintain an override list of the input file. This
 * list is fist checked by pro_getdefault. "s" should have the form:
 * section name value
 * 
 * Returns: nothing
 * 
 * Remarks: See also: @getvar@
 * */
void
setvar (char *s)
{
	char    name[NAMEL];
	char    section[SECL];
	char    value[VALL];

	if (!(getvar (s))){	/* don't allow for double entries  */
		nrvars++;
		sscanf (s, "%s %s %s", section, name, value);
		vars = (varst *) defmem ((void *) vars, (1 + nrvars) * sizeof (varst), defprog);
		vars[nrvars].section = (char *) defmem ((void *) NULL, 1 + strlen (section), defprog);
		vars[nrvars].name = (char *) defmem ((void *) NULL, 1 + strlen (name), defprog);
		vars[nrvars].value = (char *) defmem ((void *) NULL, 1 + strlen (value), defprog);
		strcpy (vars[nrvars].section, section);
		strcpy (vars[nrvars].name, name);
		strcpy (vars[nrvars].value, value);
	}
}


/*C:getvar
 *@char *getvar(char *s)
 *
 * getvar - return value of section name
 *	s should be: section name
 *	Ret: pointer to value, or NULL if not found
 *	See also: s@etvar(char *s)@
 */

char
*getvar (char *s)
{
	char    name[NAMEL];
	char    section[SECL];
	int     i;

	sscanf (s, "%s %s", section, name);


	for (i = 0; i <= nrvars; i++) {
		if ((Strcasecmp (vars[i].section, section) == 0) &&
				(Strcasecmp (vars[i].name, name) == 0))
			return vars[i].value;
	}

	return (char *) NULL;
}

char *(*editfunc)(char *editstr) = NULL;

/*Ceditmemitem
 *@void editmemitem(char *section, char *name, char *def)
 *
 * Edits the item in the memlist, returning @*def@ on escape
 * If @editfunc@ points to an edit function that function
 * is used, otherwise @fgets@ is used.
 */
void
editmemitem(char *section,char *name,char *def)
{
	char editbuf[LBUFF];

	if (editfunc != NULL){
		strcpy(editbuf,editfunc(def));
	}else{
		fprintf(stderr,"default = %s: ",def);
		fgets(editbuf,LBUFF,stdin);
		if (strlen(editbuf) < 1)
			strcpy(editbuf,def);
	}
	addvar("NO FILE DEFINED",section,name,editbuf,0);
}

/*C:rinmem
 *@int rinmem(char *fname)
 *
 * reads an intire defaults file into mem (the vars list)
 * options are:
 *@	1: dump file to stdout (removed)
 *@	2: dump file to stdout, strip comments (removed)
 *@	3: read into the override defmem list
 *@	4: read into the normal defmem list
 *
 * Returns: 0 is error, 1= ok
 */
int     opt = 4;
int check = 0;

int 
rinmem (char *fname)
{
	char   *cp, *rp;
	static char    buf[LBUFF];
	FILE   *thef;
	char    section[SECL];
	static char    name[NAMEL];
	static char    value[VALL];
	static char    outp[LBUFF];

	if (fname) {
		if (strcmp (fname, "-") == 0) {
			thef = stdin;
		} else {
			if ((thef = fopen (fname, "r")) == NULL) {
				return 0;
			}
		}
	} else
		return 0;/*  assume already opened defaults */

		while ((cp = fgets (buf, LBUFF, thef)) != (char *) NULL) {
		while (isspace (*cp))
			cp++;
		if (*cp && !strchr(commchar,*cp)) {
nsec:	/* this is a bit of a kludge, but it works */
		if (*cp == '[' && (rp = strchr (cp, ']')) !=
				(char *) NULL) {
			*rp = '\0';
			cp++;
			strcpy (section, cp);
			/*  found a section header */
			/* while ((cp=Fgets (buf, LBUFF, thef)) != (char *) NULL) { */
			while ((cp=fgetl (thef)) != (char *) NULL) {
			/* cp[strlen (cp) - 1] = '\0'; */
			while (isspace (*cp))
				cp++;
			if (*cp== '[')/*Bail out-we are at new section*/
				goto nsec;

			if (*cp && !strchr(commchar,*cp)){
				for (rp = cp; *rp && *rp != '='; rp++)
					;
				/* Skip trailing whitespace */
				while (isspace (*rp)|| *rp == '=') rp--;
				rp++;		
				if (*rp)
					*rp++ = '\0';
				strcpy (name, cp);
				/* YES! a var was found */
				if (opt == 1)
					fprintf (stdout, "%s = ", name);
				rp--;
				*rp = ' ';
				cp = rp;
				while (*cp != '=' && *cp != '\0')
					cp++;
				if (*cp != '\0')
					cp++;
				else
					cp = rp;
				while (isspace (*cp))	
					/* skip whitespace */
					cp++;
				for (rp = cp; *rp == '\n' || *rp == '\r'; rp++);
				/* go to end of line  or commentchar */
				for (rp--; !isspace (*rp); rp--);
				/* go back to last text */
				if (*rp)
					*rp++ = '\0';
				if (strlen(cp)){
				strcpy (value, cp);
				if (opt == 3) {
					sprintf (outp, "%s %s %s", section, name, value);
					setvar (outp);
				}
				else if (opt == 4) {
					if (defverb > 3)
						prit(section);
					addvar (fname,section, name, value,0);
				}
				}
			}
			}
		}
		}
		}

  if (thef != stdin)
    (void) fclose (thef);

  return 1;
}

/*C:rinmem_buf
 *@int rinmem_buf(const char *fname_key, const char *text)
 *
 * Like rinmem() but reads from a string buffer instead of a file.
 * @fname_key@ is used as the in-memory file key (same role as the filename
 * in rinmem); @text@ is the full INI content as a null-terminated string.
 * Returns 1 on success, 0 on error.
 */
int
rinmem_buf(const char *fname_key, const char *text)
{
    char   *cp, *rp;
    static char    buf[LBUFF];
    FILE   *thef;
    char    section[SECL];
    static char    name[NAMEL];
    static char    value[VALL];

    if (!fname_key || !text)
        return 0;

    /* fmemopen gives us a FILE* backed by the string — no disk I/O */
    thef = fmemopen((void *)text, strlen(text), "r");
    if (!thef)
        return 0;

    while ((cp = fgets(buf, LBUFF, thef)) != (char *) NULL) {
        while (isspace(*cp))
            cp++;
        if (*cp && !strchr(commchar, *cp)) {
nsec_buf:
            if (*cp == '[' && (rp = strchr(cp, ']')) != (char *) NULL) {
                *rp = '\0';
                cp++;
                strcpy(section, cp);
                while ((cp = fgetl(thef)) != (char *) NULL) {
                    while (isspace(*cp))
                        cp++;
                    if (*cp == '[')
                        goto nsec_buf;
                    if (*cp && !strchr(commchar, *cp)) {
                        for (rp = cp; *rp && *rp != '='; rp++)
                            ;
                        while (isspace(*rp) || *rp == '=') rp--;
                        rp++;
                        if (*rp)
                            *rp++ = '\0';
                        strcpy(name, cp);
                        rp--;
                        *rp = ' ';
                        cp = rp;
                        while (*cp != '=' && *cp != '\0')
                            cp++;
                        if (*cp != '\0')
                            cp++;
                        else
                            cp = rp;
                        while (isspace(*cp))
                            cp++;
                        for (rp = cp; *rp == '\n' || *rp == '\r'; rp++);
                        for (rp--; !isspace(*rp); rp--);
                        if (*rp)
                            *rp++ = '\0';
                        if (strlen(cp)) {
                            strcpy(value, cp);
                            if (opt == 4)
                                addvar(fname_key, section, name, value, 0);
                        }
                    }
                }
            }
        }
    }

    fclose(thef);
    return 1;
}
