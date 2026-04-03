/* $Header: /home/schj/src/vamps_0.99g/src/nr_ut.lib/RCS/nr_free.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */
 
/* $RCSfile: nr_free.c,v $
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
 "$Id: nr_free.c,v 1.2 1999/01/06 12:13:01 schj Alpha $";

#ifdef NR_LIST

extern int cldfrf;
/*C:void nr_free_substr
 *@void nr_free_substr(char *substr)
 *
 * Free all items with contain the string @substr@ in their description
 * See also @nr_descr@, @nr_free@, @nr_free_all@.
 */ 
void
nr_free_substr(char *substr)
{
	int i;

	for (i = 0; i<md_nr; i++)
		if (md[i].t != NR_EMPTY && strstr(md[i].desc,substr) != NULL)
			nr_free(1,(void *)md[i].d.dv);
}

/*C:nr_free
 *@int nr_free(int datfree, void *a)
 *
 * Free the entry with address @a@ in the @md@ structure. If
 * @datfree@ is 1 the data itself (@a@) is also freed.  Returns -1 on
 * error, otherwise the number of items freed.
 * */
int
nr_free(int datfree, void *a)
{
	int nr,nrf = 0;
	mdata_type *d;
	
	nr = get_id((void *)a);
	if (nr == -1)
		return -1;

	/* Hack to stop recursive calls, checked in free_* routines */
	cldfrf = 1;
#ifdef NR_DEBUG
	fprintf(stderr,"Freeing id %d (%s)\n",nr,md[nr].desc);
#endif
	d = &md[nr];
	
	switch (d->t){
	 case NR_DM:	 
		 if (datfree)
			 free_dmatrix(d->d.dm,d->fr,d->lr,d->fc,d->lc);
		 d->d.dm = (double **)NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 case NR_FM: 
		 if (datfree)
			 free_matrix(d->d.fm,d->fr,d->lr,d->fc,d->lc);
		 d->d.fm = (float **)NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 case NR_IM:
		 if (datfree)
			 free_imatrix(d->d.im,d->fr,d->lr,d->fc,d->lc);
		 d->d.im = (int **)NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 case NR_DV:
		 if (datfree)
			 free_dvector(d->d.dv,d->fr,d->lr);
		 d->d.dv = (double *)NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 case NR_FV:
		 if (datfree)
			 free_vector(d->d.fv,d->fr,d->lr);
		 d->d.fv = (float *)NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 case NR_IV:
		 if (datfree)
			 free_ivector(d->d.iv,d->fr,d->lr);
		 d->d.iv = (int *)NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 case NR_CV:
		 if (datfree)
			 free_cvector(d->d.cv,d->fr,d->lr);
		 d->d.cv = (unsigned char *)NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 case NR_LV:
		 if (datfree)
			 free_lvector(d->d.lv,d->fr,d->lr);
		 d->d.lv = (unsigned long *) NULL;
		 d->t = NR_EMPTY;
		 free((FREE_ARG)d->desc);
		 d->desc = NULL;
		 break;
	 default:
		 break;
	}
	
	/* Schrink list if last place is free ...*/
	if (nr == md_nr -1){
#ifdef NR_DEBUG
		fprintf(stderr,"Schrinking list (%d)\n",md_nr -1);
#endif		
		while(md[md_nr -1].t == NR_EMPTY){
#ifdef NR_DEBUG
			fprintf(stderr,"Really deleting: %d\n",md_nr -1);
#endif
			md_nr--;
			nrf++;
		}
		if (md_nr == 0){
			free(md);
			md = NULL;
		}else{
			md = (mdata_type *)realloc((mdata_type *)md,(size_t)md_nr*sizeof(mdata_type));
		}
	}

	cldfrf = 0;
	if (nrf)
		return nrf;
	else
		return 1;
}


/*C:void nr_free_all
 *@void nr_free_all(void)
 *
 * Free all matrix and vector variable allocated.
 * See also @nr_free@.
 */ 
void
nr_free_all(void)
{
	int i;

	for (i = 0; i<md_nr; i++)
		if (md[i].t != NR_EMPTY)
			nr_free(1,(void *)md[i].d.dv);
}
#endif  /* NR_LIST */
