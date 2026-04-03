#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nrutil.h"

#ifdef NR_LIST
extern int add_m(int type, void *vp, int fr, int lr, int fc, int lc, char *desc);
#endif

/* Some defines to control the output layout */
#define DOUT "%12.6f"  /* Double ascii output format */
#define FOUT "%g"      /* Float ascii output format */
#define IOUT "%d"
#define LOUT "%ld"
#define COUT "%c"
#define CMT  '#'
#define COLSEP "\t"    /* Column separation in ascii output */
#define LINESEP "\n"   /* line separation in ascii output */
#define W_OPENMODE "w" /* Open mode for writing files */


/* forward declarations */
int nr_openout(char *fname,FILE *str);
int nr_closeout(void);
int nr_whead(FILE *str,void *obj);

/*C:nroutf
 *@static FILE *nroutf
 *
 * If this file is set it is used by the nr_[df]mwrite functions.  Use
 * @nr_openout@ and @nr_closeout@ to set this variable*/
static FILE *nroutf = NULL;

/*C:nr_fgetl
 *@ char *nr_fgetl(FILE *fp)
 *
 * Get the next line from open file fp up to the newline which is 
 * replaced with NULL. Returns char * to the NULL terminated string  or
 * NULL on EOF or error; return ptr points to static memory 
 * overwritten at each invocation.  This is taken from slash by 
 * R. Venneker */
char *
nr_fgetl(FILE *fp)
{
	int	c, n;
	static int	nb = 0;
	static char	*buf;

	if(!nb)
		buf = (char *)malloc((nb = 64) * sizeof(char));
	n = 0;
	while((c = getc(fp)) != EOF && c != '\n') {
		if(n >= nb) {
			buf = (char *)realloc(buf, (nb += 64) * sizeof(char));
		}
		buf[n++] = (char)c;
	}
	if(!n && c != '\n')
		return((char *)NULL);

	buf[n] = '\0';
	return(buf);
}

/*C:nr_dmread
 *@ double **nr_dmread(char *file, int *nr, int *nc)
 *
 * Reads a matrix (2D floating point array) from the named ascii
 * @file@, which is supposed to consist of lines with numerical fields
 * separated by whitespace (spaces, TABS and/or newlines). Lines
 * starting with `@#@' or `@%@' are considered comments; blank lines
 * are skipped. If @nc@>0, the number of matrix columns are set to
 * @nc@. If @nc@ is zero or less, the number of columns are set equal
 * to the number of fields in the input lines. The number of matrix
 * rows are calculated from dividing the number of values by the
 * number of lines in the input @file@. 
 *
 * remarks: this function returns a zero based matrix.
 * 
 * This functions was adapted from mread by R. Venneker.*/
double **nr_dmread(char *name,int *nr, int *nc)
{
	int	i,all, n, m;
	char	*sp, *rp;
	double	**pp, *p, val;
	FILE	*fp;
	extern double	strtod();	/* to avoid confusion */

	n = m = 0;
	all = 32;
	if((fp = fopen(name, "r")) == (FILE *)NULL)
		return (double **)NULL;

	if((p = (double *)malloc(all * sizeof(double))) == (double *)NULL)
		l_error(1,"nr_dmread","could not allocate matrix (%d)\n",all);

	while((sp = nr_fgetl(fp)) != (char *)NULL) {
	/* check for blank or comment lines */
		if(*sp != '\0' && *sp != '#' && *sp != '%') {
			for(;;) {
				val = strtod(sp, &rp);
				if(sp == rp)
					break;
				if(n >= all) {
					p = (double *)realloc((char *)p,
						(all += 32) * sizeof(double));
					if(p == (double *)NULL)
						l_error(1,"nr_dmread","could not allocate matrix (%d)\n",all);
				}
				p[n++] = val;
				sp = rp;
			}
			m++;
		}
	}
	(void)fclose(fp);
	if(*nc > 0)
		m = n / *nc;

	if (m == 0){
		free((FREE_ARG)p);
		*nc= 0; *nr= 0;
		return (double **)NULL;
	}

	n /= m;
	*nc = n;
	*nr = m;

	p = (double *)realloc(p, n * m * sizeof(double));
	if(p == (double *)NULL)
		l_error(1,"nr_dmread","could not allocate matrix (%d)\n",n*m);

	pp = (double **)malloc((size_t) m * sizeof(double *));
	if (pp == (double **)NULL)
		l_error(1,"nr_dmread","could not allocate matrix (%d)\n",m);
	
	pp[0] = p;
	for (i=1;i<m;i++)
		pp[i] = pp[i-1]+n;

#ifdef NR_LIST
	add_m(NR_DM, (void *)pp, 0, m - 1, 0, n - 1, name);
#endif
	return pp;
}

/*C:nr_fmread
 *@ float **nr_fmread(char file, int *nr, int *nc)
 *
 * Reads a matrix (2D floating point array) from the named ascii
 * @file@, which is supposed to consist of lines with numerical fields
 * separated by whitespace (spaces, TABS and/or newlines). Lines
 * starting with `@#@' or `@%@' are considered comments; blank lines
 * are skipped. If @nc@>0, the number of matrix columns are set to
 * @nc@. If @nc@ is zero or less, the number of columns are set equal
 * to the number of fields in the input lines. The number of matrix
 * rows are calculated from dividing the number of values by the
 * number of lines in the input @file@.  
 *
 * remarks: this function returns a zero based matrix.
 *
 * This functions was adapted from mread by R. Venneker.*/
float **nr_fmread(char *name,int *nr, int *nc)
{
	int	i,all, n, m;
	char	*sp, *rp;
	float	**pp, *p, val;
	FILE	*fp;
	extern double	strtod();	/* to avoid confusion */

	n = m = 0;
	all = 32;
	if((fp = fopen(name, "r")) == (FILE *)NULL)
		return (float **)NULL;

	if((p = (float *)malloc(all * sizeof(float))) == (float *)NULL)
		l_error(1,"nr_fmread","could not allocate matrix (%d)\n",all);

	while((sp = nr_fgetl(fp)) != (char *)NULL) {
	/* check for blank or comment lines */
		if(*sp != '\0' && *sp != '#' && *sp != '%') {
			for(;;) {
				val = (float)strtod(sp, &rp);
				if(sp == rp)
					break;
				if(n >= all) {
					p = (float *)realloc((char *)p,
						(all += 32) * sizeof(float));
					if(p == (float *)NULL)
						l_error(1,"nr_fmread","could not allocate matrix (%d)\n",all);
				}
				p[n++] = val;
				sp = rp;
			}
			m++;
		}
	}
	(void)fclose(fp);
	if(*nc > 0)
		m = n / *nc;

	if (m == 0){
		free((FREE_ARG)p);
		*nc= 0; *nr= 0;
		return (float **)NULL;
	}

	n /= m;
	*nc = n;
	*nr = m;

	p = (float *)realloc(p, n * m * sizeof(float));
	if(p == (float *)NULL)
		l_error(1,"nr_fmread","could not allocate matrix (%d)\n",n*m);

	pp = (float **)malloc((size_t) m * sizeof(float *));
	if (pp == (float **)NULL)
		l_error(1,"nr_fmread","could not allocate matrix (%d)\n",m);
	
	pp[0] = p;
	for (i=1;i<m;i++)
		pp[i] = pp[i-1]+n;

#ifdef NR_LIST
	add_m(NR_FM, (void *)pp, 0, m - 1, 0, n - 1, name);
#endif
	return pp;
}

/*C:nr_dmwrite
 *@int nr_dmwrite(char *fname, double **m, int fr, int lr, int fc, int lc)
 *
 * Write the matrix @**m@ to the file @fname@.
 *
 * Returns -1 on error and 0 on success.
 *
 * Remarks: If the global @nroutf@ variable is set that file will be
 * used for output, assuming it is already open. See also @nr_openout@
 * and @nr_closeout@.  Otherwise the file is opened in @W_OPENMODE@
 * mode, so it should be possible to write a descriptive header first.
 * */
int nr_dmwrite(char *fname, double **m, int fr, int lr, int fc, int lc)
{
	int i,j;
	FILE *outf = NULL;

	if (!m)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i=fr; i<= lr; i++){
		for (j=fc;j<=lc;j++){
			(void) fprintf(outf,DOUT,m[i][j]);
			if (j<lc)
				(void) fprintf(outf,COLSEP);
		}
		(void) fprintf(outf,LINESEP);
	}
	
	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

/*C:nr_imwrite
 *@int nr_imwrite(char *fname, int **m, int fr, int lr, int fc, int lc)
 *
 * Write the integer matrix @**m@ to the file @fname@.
 *
 * Returns -1 on error and 0 on success.
 *
 * Remarks: See @nr_dmread@.  */
int nr_imwrite(char *fname, int **m, int fr, int lr, int fc, int lc)
{
	int i,j;
	FILE *outf = NULL;

	if (!m)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i=fr; i<= lr; i++){
		for (j=fc;j<=lc;j++){
			(void) fprintf(outf,IOUT,m[i][j]);
			if (j<lc)
				(void) fprintf(outf,COLSEP);
		}
		(void) fprintf(outf,LINESEP);
	}
	
	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

#ifdef NR_LIST
/*C:nr_genw
 *@int nr_genw(char *fname, void *obj)
 *
 * Writes an ascii representation of an object (matrix, vector)
 * to a file @fname@. This is a wrapper for the @nr_w*@ functions
 * using the @md@ structure and the @get_id@ function.
 *
 * Returns -1 on error (pointer not in list) and 0 on success.
 */
int
nr_genw(char *fname, void *obj)
{
	int nr;
	int rval = -1;
	mdata_type *d;

	nr = get_id((void *)obj);
	if (nr == -1)
		return -1;

	d = &md[nr];

	nr_openout(fname,NULL);	
	/* Write header info ...*/
	nr_whead(nroutf,(void *)obj);
	switch (d->t){
		case NR_DM:	 
			rval = nr_dmwrite(fname, d->d.dm, d->fr, d->lr, d->fc, d->lc);
			break;
		case NR_FM: 
			rval = nr_fmwrite(fname, d->d.fm, d->fr, d->lr, d->fc, d->lc);
			break;
		case NR_IM:
			rval = nr_imwrite(fname, d->d.im, d->fr, d->lr, d->fc, d->lc);
			break;
		case NR_DV:
			rval = nr_dvwrite(fname, d->d.dv, d->fr, d->lr);
			break;
		case NR_FV:
			rval = nr_fvwrite(fname, d->d.fv, d->fr, d->lr);
			break;
		case NR_IV:
			rval = nr_ivwrite(fname, d->d.iv, d->fr, d->lr);
			break;
		case NR_CV:
			rval = nr_cvwrite(fname, d->d.cv, d->fr, d->lr);
			break;
		case NR_LV:
			rval = nr_lvwrite(fname, d->d.lv, d->fr, d->lr);
			break;
		default:
			break;
	}
	(void)fflush(nroutf);
	(void) nr_closeout();	
	return rval;
}

/*C:nr_whead
 *@ int nr_whead(FILE *str,void *obj)
 *
 * Writes header information of @obj@ to stream @str@ which should
 * be open for writing.
 */ 
int
nr_whead(FILE *str,void *obj)
{
	mdata_type *d;
	int	nr;

	nr = get_id((void *)obj);
	if (nr == -1)
		return -1;
	else{
		d = &md[nr];

#ifdef NOOCTAVE		
		(void)fprintf(str,"%c Description: %s\n",CMT,d->desc);
		switch (d->t){
			case NR_DM:	 
				(void)fprintf(str,"%c Type: double matrix\n",CMT);
				(void)fprintf(str,"%c Firstcol: %d\n",CMT,d->fc);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastcol: %d\n",CMT,d->lc);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_FM: 
				(void)fprintf(str,"%c Type: float matrix\n",CMT);
				(void)fprintf(str,"%c Firstcol: %d\n",CMT,d->fc);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastcol: %d\n",CMT,d->lc);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_IM:
				(void)fprintf(str,"%c Type: integer matrix\n",CMT);
				(void)fprintf(str,"%c Firstcol: %d\n",CMT,d->fc);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastcol: %d\n",CMT,d->lc);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_DV:
				(void)fprintf(str,"%c Type: double vector\n",CMT);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_FV:
				(void)fprintf(str,"%c Type: float vector\n",CMT);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_IV:
				(void)fprintf(str,"%c Type: integer vector\n",CMT);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_CV:
				(void)fprintf(str,"%c Type: character vector\n",CMT);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_LV:
				(void)fprintf(str,"%c Type: long vector\n",CMT);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			default:
				return -1;
				break;
		}
#else
		(void)fprintf(str,"%c name: %s\n",CMT,d->desc);
		switch (d->t){
			case NR_DM:	 
				(void)fprintf(str,"%c type: matrix\n",CMT);
				(void)fprintf(str,"%c rows: %d\n",CMT,d->lc-d->fc);
				(void)fprintf(str,"%c columns: %d\n",CMT,d->lr-d->fr);
				break;
			case NR_FM: 
				(void)fprintf(str,"%c type: matrix\n",CMT);
				(void)fprintf(str,"%c rows: %d\n",CMT,d->lc-d->fc);
				(void)fprintf(str,"%c columns: %d\n",CMT,d->lr-d->fr);
				break;
			case NR_IM:
				(void)fprintf(str,"%c type: matrix\n",CMT);
				(void)fprintf(str,"%c rows: %d\n",CMT,d->lc-d->fc);
				(void)fprintf(str,"%c columns: %d\n",CMT,d->lr-d->fr);
				break;
			case NR_DV:
				(void)fprintf(str,"%c type: matrix\n",CMT);
				(void)fprintf(str,"%c rows: %d\n",CMT,d->lr - d->fr);
				(void)fprintf(str,"%c columns: %d\n",CMT,1);
				break;
			case NR_FV:
				(void)fprintf(str,"%c type: matrix\n",CMT);
				(void)fprintf(str,"%c rows: %d\n",CMT,d->lr - d->fr);
				(void)fprintf(str,"%c columns: %d\n",CMT,1);
				break;
			case NR_IV:
				(void)fprintf(str,"%c type: matrix\n",CMT);
				(void)fprintf(str,"%c rows: %d\n",CMT,d->lr - d->fr);
				(void)fprintf(str,"%c columns: %d\n",CMT,1);
				break;
			case NR_CV:
				(void)fprintf(str,"%c Type: character vector\n",CMT);
				(void)fprintf(str,"%c Firstrow: %d\n",CMT,d->fr);
				(void)fprintf(str,"%c Lastrow: %d\n",CMT,d->lr);
				break;
			case NR_LV:
				(void)fprintf(str,"%c type: matrix\n",CMT);
				(void)fprintf(str,"%c rows: %d\n",CMT,d->lr - d->fr);
				(void)fprintf(str,"%c columns: %d\n",CMT,1);
				break;
			default:
				return -1;
				break;
		}
#endif
	}

	return 0;
}
#endif /* NR_LIST */

/*C:nr_fmwrite
 *@ int nr_fmwrite(char *fname, float **m, int fr, int lr, int fc, int lc)
 *
 * Write the matrix @**m@ to the file @fname@.
 *
 * Returns -1 on error and 0 on success.
 *
 * Remarks: See @nr_dmread@. 
 *  */
int nr_fmwrite(char *fname, float **m, int fr, int lr, int fc, int lc)
{
	int i,j;
	FILE *outf = NULL;

	if (!m)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i=fr; i<=lr; i++){
		for (j=fc;j<=lc;j++){
			(void) fprintf(outf,FOUT,m[i][j]);
			if (j<lc)
				(void) fprintf(outf,COLSEP);
		}
		(void) fprintf(outf,LINESEP);
	}
	
	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

/*C:nr_dvwrite
 *@ int nr_dvwrite(char *fname, double *v, int fr, int lr)
 *
 * writes a double vector @v@ to the file @fname@.
 */
int
nr_dvwrite(char *fname, double *v, int fr, int lr)
{
	int i;
	FILE *outf = NULL;

	if (!v)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i = fr; i <=lr; i++){
			(void) fprintf(outf,DOUT,v[i]);
			(void) fprintf(outf,"\n");
	}

	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

/*C:nr_ivwrite
 *@ int nr_ivwrite(char *fname, double *v, int fr, int lr)
 *
 * writes a int vector @v@ to the file @fname@.
 */
int
nr_ivwrite(char *fname, int *v, int fr, int lr)
{
	int i;
	FILE *outf = NULL;

	if (!v)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i = fr; i <=lr; i++){
			(void) fprintf(outf,IOUT,v[i]);
			(void) fprintf(outf,"\n");
	}

	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

/*C:nr_lvwrite
 *@ int nr_lvwrite(char *fname, unsigned long *v, int fr, int lr)
 *
 * writes a long vector @v@ to the file @fname@.
 */
int
nr_lvwrite(char *fname,unsigned  long *v, int fr, int lr)
{
	int i;
	FILE *outf = NULL;

	if (!v)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i = fr; i <=lr; i++){
			(void) fprintf(outf,LOUT,v[i]);
			(void) fprintf(outf,"\n");
	}

	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

/*C:nr_cvwrite
 *@ int nr_cvwrite(char *fname,unsigned  char *v, int fr, int lr)
 *
 * writes a char vector @v@ to the file @fname@.
 */
int
nr_cvwrite(char *fname,unsigned  char *v, int fr, int lr)
{
	int i;
	FILE *outf = NULL;

	if (!v)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i = fr; i <=lr; i++){
			(void) fprintf(outf,COUT,v[i]);
			(void) fprintf(outf,"\n");
	}

	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

/*C:nr_fvwrite
 *@ int nr_fvwrite(char *fname, float *v, int fr, int lr)
 *
 * writes a float vector @v@ to the file @fname@.
 */
int
nr_fvwrite(char *fname, float *v, int fr, int lr)
{
	int i;
	FILE *outf = NULL;

	if (!v)
		return -1;

	if (nroutf)
		outf = nroutf;
	else
		if((outf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;

	for (i = fr; i <=lr; i++){
			(void) fprintf(outf,FOUT,v[i]);
			(void) fprintf(outf,"\n");
	}

	if(outf != nroutf)
		(void) fclose(outf);

	return 0;
}

int
nr_openout(char *fname,FILE *str)
{
	if (nroutf)
		nr_closeout();

	if (str){
		nroutf = str;
	}else{
		if((nroutf = fopen(fname, W_OPENMODE)) == (FILE *)NULL)
			return -1;
	}

	return 0;
}

/*
 * Returns 0 on succes, EOF on failure
 * */
int
nr_closeout(void)
{
	int retval; 

	if (!nroutf)
		return EOF;
	
	retval = fclose(nroutf);

	nroutf = NULL;

	return retval;
}
