/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/alloc.c,v 1.9 1999/01/06 12:13:01 schj Alpha $ */
/*  
 *  $RCSfile: alloc.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DDEBUG
static  char RCSid[] =
"$Id: alloc.c,v 1.9 1999/01/06 12:13:01 schj Alpha $";
#endif

#include "swatsoil.h"
#include "nrutil.h"
#include "marquard.h"


/*Callocall
 *@ void allocall (int layers)
 * Description: allocate arrays and data-sets needed in swatsoil.c
 * Returns: nothing
 * Remarks: Should be cleaned
 */

void
allocall (layers)
     int layers;
{
	int i;


	/* allocate memory for the layers */
	node = (node_t *) ts_memory ((void *) NULL, 
			layers * sizeof (node_t), progname);

	/* Allocate data sets for output */
	if ((id.qbo = getsetbyname("qbo")) == -1){
		add_set (NULL,"qbo","qbo",steps,0,0);
		id.qbo = getsetbyname("qbo");
	}

	id.vol = add_set (NULL,"vol","vol",steps,0,0);
	id.avt = add_set (NULL,"avt","avt",steps,0,0);
		
	theta = dvector(0, layers -1);
	nr_descr("theta at each dt",(void *)theta);
	howsat = dvector(0, layers -1);
	nr_descr("perc saturation",(void *)howsat); 
	diffmoist = dvector(0, layers -1);
	thetm1 = dvector(0, layers -1);
 	dzc = dvector(0, layers -1); 
 	dzf = dvector(0, layers -1); 
 	dz = dvector(0, layers -1); 
	z = dvector(0, layers -1);
	hm1 = dvector(0, layers -1);
	h = dvector(0, layers -1);
	qrot = dvector(0, layers -1);
	depth = dvector(0, layers);
	k = dvector(0, layers -1);
	kgeom = dvector(0, layers);
	q = dvector(0, layers);
	inq = dvector(0, layers);

	qdra = dmatrix (0,layers, 0, 3);
	nr_descr("qdra, lateral drainage",(void *)qdra);
	basegw = dvector(0,1);

	error = ivector (0, steps + 1);
	itter = ivector (0, steps + 1);
	gwl = dvector (0, steps + 1);
	allowdrain = ivector (0, layers -1);

	/* Filling with initial values */
	for (i = 0; i < layers; i++){
		depth[i] = 0.0;
		allowdrain[i] = 1;
		qdra[0][i] = 0.0;
		qdra[1][i] = 0.0;
		qdra[2][i] = 0.0;
		qdra[3][i] = 0.0;
	}
	/* These arrays cover the entire time domain */  
	for (i = 0; i < steps + 2; i++){
		error[i] = 0;
		itter[i] = 0;
	}
}
