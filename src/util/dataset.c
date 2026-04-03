/* $Header: /home/schj/src/vamps_0.99g/src/util/RCS/dataset.c,v 1.20 1999/01/06 12:13:01 schj Alpha $ */

/*  $RCSfile: dataset.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "vamps.h"
#include "deffile.h"

static char RCSid[] =
"$Id: dataset.c,v 1.20 1999/01/06 12:13:01 schj Alpha $";

#ifdef PATH_MAX
#define MAXPATHLEN PATH_MAX
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN 78
#endif

ID	id;
dataset *data = (dataset *) NULL;	/* data sets */
int sets = 0;			/* number of datasets */
char outdir[MAXPATHLEN];

/* --- In-memory forcing array registry (used by Python extension) --- */
#define VAMPS_TS_MAX 64
typedef struct { char name[64]; double *vals; int n; double firststep; } vamps_ts_arr_t;
static vamps_ts_arr_t vamps_ts_reg[VAMPS_TS_MAX];
static int vamps_ts_n = 0;

void ts_register_array(const char *name, double *vals, int n, double firststep) {
    if (vamps_ts_n < VAMPS_TS_MAX) {
        strncpy(vamps_ts_reg[vamps_ts_n].name, name, 63);
        vamps_ts_reg[vamps_ts_n].name[63] = '\0';
        vamps_ts_reg[vamps_ts_n].vals = vals;
        vamps_ts_reg[vamps_ts_n].n = n;
        vamps_ts_reg[vamps_ts_n].firststep = firststep;
        vamps_ts_n++;
    }
}

void ts_clear_registry(void) { vamps_ts_n = 0; }

static vamps_ts_arr_t *ts_find_arr(const char *name) {
    int i;
    for (i = 0; i < vamps_ts_n; i++)
        if (strcmp(vamps_ts_reg[i].name, name) == 0)
            return &vamps_ts_reg[i];
    return NULL;
}

/*
typedef struct {
	char *setname;
	char *desc;
	char *unit;
} st;

st validsets[] = {
	{"pre","precipitation", "cm" },
	{"rlh",	"relative humidity","%"},
	{"hea","head at bottom","cm"},
	{"rdp","rooting depth","cm"},
	{"tem","dry bulb temp","oC"},
	{"gwt","ground-water table","cm"}
}
	inr;	interception	[cm] 
	trf;	troughfall	[cm]
	stf;	stemflow	[cm]
	pev;	potential evaporation [cm]
	spe;	potential soil evaporation [cm]
	ptr;	potential transpiration [cm]
	qbo;	flow through bottom of profile [cm]
	vol;	actual water content [cm]
	avt;	average theta in profile
	fit;	set in fitting
	lai;	leaf area index
	sca;	canopy storage [cm???]
	ira;	incoming radiation [W/m2]
	nra;	net radiation [W/m2]
	ref;	reflected radiation [W/m2]
	win;	windspeed [m/s]
	sur;	sunratio (n/N)
*/

/*C:getsetbyname 
 *@ int getsetbyname(char *s)
 *
 * returns the number of a named set
 * searches for a set named name, returns -1 on error
 * empty positions (xy = NULL ) are skipped
 */
int
getsetbyname (char *s)
{
	int i;

	if (sets > 0)
		for (i = 0; i < sets; i++){
			if (strcmp (s, data[i].name) == 0 && data[i].xy != NULL)
				return i;
		}

	return -1;
}

/*C:del_set
 *@ int del_set(int nr)
 *
 * deletes dataset from mem and list and frees the memory allocted to the
 * actual data. Use @getsetbyname()@ to find the sets
 *
 * The entry in the data struct will remain but the pointer to xy will be
 * set to @NULL@, indicating the place is free. 
 */
int
del_set (int nr)
{
	char cc[1024];

	sprintf (cc, "deleting set %d (%s)", nr, data[nr].name);
	showit ("del_set",MESG,cc,2,verbose);
  
	free (data[nr].xy);
	data[nr].xy = NULL;

	strcpy (data[nr].fname, "deleted");
	strcpy (data[nr].name, "deleted");
	data[nr].points = 0;
	data[nr].vunit = -1;
	data[nr].tunit = -1;
	data[nr].xsum = MISSVAL;
	data[nr].ysum = MISSVAL;
	data[nr].xmax = MISSVAL;
	data[nr].ymax = MISSVAL;
	data[nr].ymin = MISSVAL;
	data[nr].xmin = MISSVAL;

	return 0;
}

/*C:dell_all_sets
 *@ void dell_all_sets()
 *
 * deletes all datasets from memory
 */
void
del_all_sets ()
{
	int i;

	if (sets > 0){
		for (i = sets-1; i >= 0; i--){
			if (verbose > 1)
				fprintf (stderr, "dataset.c: deleting set %d: %s\n", i, data[i].name);
			if (data[i].xy) /* could be an empty pos */
				free (data[i].xy);
		}
		free (data);
		data = NULL;
		sets = 0;
	}
}


/*C:add_set
 *@ int add_set(xy *XY,char *name,char *fname,int points,int vunit,int tunit);
 * 
 * Adds an XY structure to the data structure and list returns the number
 * of the set made. If @xy == NULL@ the XY structure is allocate with
 * @points@ points. x and Y values are filled with zeros.
 *
 */
int
add_set (XY *xy,char  *name,char *fname,int  points,int vunit,int tunit)
{
	int i,j;
	char cc[1024];
	XY *tmpxy = NULL;

	j = 0;

	sprintf (cc, "set %s with fname %s, points %d, vunit %d, tunit %d",
			name, fname, points, vunit, tunit);
	showit ("add_set",MESG,cc,2,verbose);

	/* allocate memory if xy is empty */
	if (xy == NULL){
	showit ("add_set",MESG,"allocating memory",2,verbose);
		tmpxy = (XY *)ts_memory ((void *) NULL,
				points * sizeof (XY), progname);
		/* Fill with values if id.pre present */
			for (i = 0; i < points; i++){
				tmpxy[i].x = 0.0;
				tmpxy[i].y = 0.0;
			}
	}
	showit ("add_set",MESG,"using pre-allocated memory",2,verbose);

	if (sets <= 0)
		data = (dataset *)
			ts_memory ((void *) data, sizeof (dataset), progname);

	if ((j = getsetbyname (name)) > 0){ /* update existing set with same name */
		showit ("add_set",WARN,"updating existing set",2,verbose);
		free(data[j].xy);
		if (xy != NULL)
			data[j].xy = xy;
		else
			data[j].xy = tmpxy;
		if (name)
			strcpy (data[j].name, name);
		if (fname)
			strcpy (data[j].fname, fname);
		data[j].points = points;
		data[j].vunit = vunit;
		data[j].tunit = tunit;
		data[j].xsum = MISSVAL;
		data[j].ysum = MISSVAL;
		data[j].xmax = MISSVAL;
		data[j].ymax = MISSVAL;
		data[j].ymin = MISSVAL;
		data[j].xmin = MISSVAL;
	}else{
		/* first check for an empty set */
		for (i=0; i<sets; i++)
			if (data[i].xy == NULL)
				break;

		/* no free pos, let data grow */
		if ( i == sets ){
			sets++;
			j = sets - 1;
			showit ("add_set",MESG,"creating new set",2,verbose);
			data = (dataset *)
				ts_memory ((void *) data, sets * sizeof (dataset), progname);

		}else{/* fill in free pos */
			j = i;
			showit ("add_set",WARN,"creating new set (old pos, untested!!)",2,verbose);
		}
			
	
		if (xy !=NULL)
			data[j].xy = xy;
		else
			data[j].xy = tmpxy;
		strcpy (data[j].name, name);
		if (fname)
			strcpy (data[j].fname, fname);
		else
			strcpy (data[j].fname, data[j].name);
		data[j].points = points;
		data[j].vunit = vunit;
		data[j].tunit = tunit;
		data[j].xsum = MISSVAL;
		data[j].ysum = MISSVAL;
		data[j].xmax = MISSVAL;
		data[j].ymax = MISSVAL;
		data[j].ymin = MISSVAL;
		data[j].xmin = MISSVAL;
	}

	return j;
}

/*C:mksetstats
 *@ void mksetstats(int dset)
 * 
 * Fill the simple statistics fields in dataset @dset@. These fields are:
 * xmin
 * xmax
 * ymin
 * ymax
 * xsum
 * ysum
 */ 
void
mksetstats(int dset)
{
	int i;

	data[dset].xsum =0.0; data[dset].ysum =0.0;
	data[dset].ymax = -1.0 * DBL_MAX; data[dset].ymin = DBL_MAX;
	data[dset].xmax = -1.0 * DBL_MAX, data[dset].xmin = DBL_MAX;

	for (i = 0; i < data[dset].points; i++){
		data[dset].xmin = data[dset].xmin > data[dset].xy[i].x ? data[dset].xy[i].x : data[dset].xmin;
		data[dset].ymin = data[dset].ymin > data[dset].xy[i].y ? data[dset].xy[i].y : data[dset].ymin;
		data[dset].ymax = data[dset].ymax < data[dset].xy[i].y ? data[dset].xy[i].y : data[dset].ymax;
		data[dset].xmax = data[dset].xmax < data[dset].xy[i].x ? data[dset].xy[i].x : data[dset].xmax;
		data[dset].xsum += data[dset].xy[i].x;
		data[dset].ysum += data[dset].xy[i].y;
	}
}

/*C:get_data
 *@ int get_data(char *fname,char *name,int minpt)
 *
 * Gets data_point(s) from file fname. The data will be added to the
 * dataset stuff using @add_set@. Columns can be given at the end of the
 * filename separated by comma's. i.e. @afilename,1,3@ would tell
 * @get_data@ the get the data from afilename and use column 1 for X and
 * column 3 for Y.
 *
 * If @minpt > 0@ then the function will exit if less then minpt are read
 * from @fname@.
 * 
 * Returns: number of points read
*/

int
get_data (char *fname,char *name,int minpt)
{
	int i = 0;

	/* Check in-memory override registry first (used by Python extension) */
	{
		vamps_ts_arr_t *ov = ts_find_arr(name);
		if (ov != NULL) {
			XY *xy = (XY *)ts_memory(NULL, ov->n * sizeof(XY), progname);
			for (i = 0; i < ov->n; i++) {
				xy[i].x = ov->firststep + i;
				xy[i].y = ov->vals[i];
			}
			mksetstats(add_set(xy, name, fname, ov->n, 0, 0));
			return ov->n;
		}
	}
	i = 0;
#ifdef NEWSET
	int nc;
	double **rc;
#endif
	int xcol = 0,ycol = 1;
	FILE *datfile;
	char s[1024];
	XY *tmpxy = (XY *) NULL;
	int nr = 0;
	char **strv;
	char *ll;
	xcol = 0;
	ycol = 1;
	strv = (char **)malloc(1 * sizeof(char *));
	if((strv[0] = strdup(fname)) == NULL)
		perror(RCSid);
	if( (ll = strchr(strv[0],',')) != NULL)
		ll[0] = '\0';

	/* Get from seperate file using the ts_readf function */
	if ((datfile = fopen (strv[0], "r")) == NULL){
		Perror (progname, 1,1, RCSid, "Error opening:", strv[0]);
	}

#ifdef NEWSET
	rc = nr_dmread(strv[0],&i,&nc);
#endif	

	(void)sprintf(s,"getting data from %s",fname);
	showit ("get_data",MESG,s,1,verbose);
	tmpxy = ts_readf (&i, datfile, "vamps",xcol,ycol, verbose);
	if (i < minpt && minpt > 0)
		Perror (progname, 1,0, RCSid, "To little points in:", strv[0]);
	fclose (datfile);
	mksetstats(add_set (tmpxy, name, fname, i, 0, 0));

	free(strv[0]);
	free(strv);


      return i;
}


int header = TRUE;

/*C:dumpset
 *@ int dumpset (int nr, FILE *stream)
 *
 * Dumps set @nr@ to stream @stream@.
 */

int
dumpset (int nr,FILE  *stream)
{
	int i;
	int check = 0;
	char s[1024];
	char dumpname[1024];

	if (nr < 0 || nr >= sets)	/* check for unrealistic values */
		return 0;

	if (!stream){	/* Generate filename from fname of dataset */
		if (data[nr].fname){
			strcpy (dumpname, data[nr].fname);
		}else{
			strcpy (dumpname, data[nr].name);
		}
		if ((stream = fopen (dumpname, "w")) == NULL){
			stream = stdout;
			strcpy (dumpname, "stdout");
		}else
			check++;
	}
	else
		strcpy (dumpname, "???");

	sprintf (s, "dumping set %s to file %s", data[nr].name, dumpname);
	showit ("dumpset",MESG,s,1,verbose);

	/* determine some set statistics if they are no set*/
	if (data[nr].xmax == MISSVAL)
		mksetstats(nr);

	/*first print a small header */
	if (header){
		(void) fprintf (stream, "%cname: %s\n", commchar[0], data[nr].name);
		(void) fprintf (stream, "%cfname: %s\n", commchar[0], data[nr].fname);
		(void) fprintf (stream, "%cpoints: %d\n", commchar[0], data[nr].points);
		(void) fprintf (stream, "%ctime unit: %d\n", commchar[0], data[nr].tunit);
		(void) fprintf (stream, "%cvalue unit: %d\n", commchar[0], data[nr].vunit);
		(void) fprintf (stream, "%cxmax: %f\n", commchar[0], data[nr].xmax);
		(void) fprintf (stream, "%cxmin: %f\n", commchar[0], data[nr].xmin);
		(void) fprintf (stream, "%cymin: %f\n", commchar[0], data[nr].ymin);
		(void) fprintf (stream, "%cymax: %f\n", commchar[0], data[nr].ymax);
		(void) fprintf (stream, "%cysum: %f\n", commchar[0], data[nr].ysum);
		(void) fprintf (stream, "%cxsum: %f\n", commchar[0], data[nr].xsum);
	}

	for (i = startpos; i < data[nr].points; i++){ /* dump the actual points */
		(void) fprintf (stream, "%f %f\n", data[nr].xy[i].x, data[nr].xy[i].y);
	}

	if (check)
		fclose (stream);

	return 1;
}
