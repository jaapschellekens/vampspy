/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/det_hatm.c,v 1.4 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: det_hatm.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */

#ifdef DDEBUG
static  char RCSid[] =
"$Id: det_hatm.c,v 1.4 1999/01/06 12:13:01 schj Alpha $";
#endif
#include <math.h>
#include "swatsoil.h"


/*C:det_hatm
 *@ double det_hatm(int i)
 *  Description: gets  hatm for step i
 *  Returns: nothing
 *  Remarks: hatm calculation kind of stupid
 */
double
det_hatm(int i)
{
	return  4698.0*22.5*log(0.6);
}
