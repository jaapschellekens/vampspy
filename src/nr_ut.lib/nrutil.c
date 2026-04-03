/* $Header: /home/schj/src/vamps_0.99g/src/nr_ut.lib/RCS/nrutil.c,v 1.4 1999/01/06 12:13:01 schj Alpha $ */
 
/* $RCSfile: nrutil.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

/* nrutils public code. Adapted in several ways 
 *
 * J. Schellekens Feb, 1997
 * */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "nrutil.h"

static char RCSid[] =
 "$Id: nrutil.c,v 1.4 1999/01/06 12:13:01 schj Alpha $";

#ifdef NR_LIST
mdata_type *md = NULL;
int md_nr = 0; /* number of structures */
int cldfrf = 0;
#endif


#ifdef NR_MEM_DEBUG
unsigned long nr_dm = 0;
unsigned long nr_m = 0;
unsigned long nr_im = 0;
unsigned long nr_dv = 0;
unsigned long nr_v = 0;
unsigned long nr_iv = 0;
void prmeminfo()
{
	fprintf(stderr,"nr_dm = %ld\n",nr_dm);
	fprintf(stderr,"nr_m = %ld\n",nr_m);
	fprintf(stderr,"nr_im = %ld\n",nr_im);
	fprintf(stderr,"nr_dv = %ld\n",nr_dv);
	fprintf(stderr,"nr_v = %ld\n",nr_v);
	fprintf(stderr,"nr_iv = %ld\n",nr_iv);
}
#endif

#ifdef NR_LIST
static int
first_empty()
{
	int i;

	for (i =0; i<md_nr; i++)
		if (md[i].t == NR_EMPTY)
			return i;

	return -1;
}

int
get_id(void *dat)
{
	int i;

	if(dat){ /* if this is not zero try to find
		 * this pointer in the list */
		for (i =0; i< md_nr;i++){
			if (dat == (void *)md[i].d.dv && md[i].t != NR_EMPTY)
				return i;
		}
	}
	if(dat){ /* if this is not zero try to find
		 * the pointer in the list with original data */
		for (i =0; i< md_nr;i++){
			if (dat == (void *)md[i].o.dv && md[i].t != NR_EMPTY){
				return i;
			}
		}
	}

	return -1;
}

int
get_id_byname(char *name)
{
	int i;

	if(name){ /* if this is not zero try to find
		 * this pointer in the list */
		for (i =0; i< md_nr;i++){
			if (strcmp(name,md[i].desc) == 0 && md[i].t != NR_EMPTY)
				return i;
		}
	}

	return -1;
}


/*C:nr_descr
 *@ int nr_descr(char *desc, void *a)
 *
 * Describe pointer @a@ with desc. If the pointer is not in the list -1 is
 * returned, otherwise 0;
 * */
int nr_descr(char *desc, void *a)
{
	int nr;

	nr = get_id((void *)a);
	if (nr == -1)
		return -1;
	else{
		if (desc){
			if (md[nr].desc)
				free(md[nr].desc);

			if((md[nr].desc =  strdup(desc)) == NULL)
				perror(RCSid);
		}
	}

#ifdef NR_DEBUG
	fprintf(stderr,"Described %d with: %s\n",nr,desc);
#endif
	return 0;
}

/*C:add_m
 *@int add_m(int type, void *vp, int fr, int lr, int fc, int lc, char *desc)
 *
 * Adds a pointer to the list. for use by the matrix and vector allocation
 * functions
 */
int
add_m(int type, void *vp, int fr, int lr, int fc, int lc, char *desc)
{
	mdata_type *d;
	int item = -1;

	if ((item = first_empty()) == -1){
#ifdef NR_DEBUG
		fprintf(stderr,"Adding new item %d (%s)\n",md_nr,desc);
#endif
		md_nr++;
		/* First allocate memory */
		if (md_nr == 1){
			md = (mdata_type *)malloc((size_t)sizeof(mdata_type));
		}else{
			md = (mdata_type *)realloc((mdata_type *)md,(size_t)md_nr*sizeof(mdata_type));
		}
		item = md_nr -1;
	}
#ifdef NR_DEBUG
	else
		fprintf(stderr,"updating item %d %s\n",item,desc);
#endif

	d = &md[item];
	d->points = d->lr - d->fr +1;
	d->cols = d->lc - d->fc +1;
	switch (type){
		case NR_DM:	 
			d->d.dm = (double **)vp;
			d->o.dm = d->d.dm[0];
			d->t = NR_DM;
		        d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		case NR_FM: 
			d->d.fm = (float **)vp;
			d->o.fm = d->d.fm[0];
			d->t = NR_FM;
			d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		case NR_IM:
			d->d.im = (int **)vp;
			d->o.im = d->d.im[0];
			d->t = NR_IM;
			d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;		
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		case NR_DV:
			d->d.dv = (double *)vp;
			d->o.dv = d->d.dv;
			d->t = NR_DV;
		        d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		case NR_FV:
			d->d.fv = (float *)vp;
			d->o.fv = d->d.fv;
			d->t = NR_FV;
        		d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		case NR_IV:
			d->d.iv = (int *)vp;
			d->o.iv = d->d.iv;
			d->t = NR_IV;
        		d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		case NR_CV:
			d->d.cv = (unsigned char *)vp;
			d->o.cv = d->d.cv;
			d->t = NR_CV;
        		d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		case NR_LV:
			d->d.lv = (unsigned long *)vp;
			d->o.lv = d->d.lv;
			d->t = NR_LV;
		        d->fc = fc;
		        d->lc = lc;
			d->fr = fr;
		        d->lr = lr;
			if (desc)
				d->desc=strdup(desc);
			else
				d->desc=strdup("(none)");
			break;
		default:
			break;
	}

	return item;
}

#endif  /* NR_LIST */
