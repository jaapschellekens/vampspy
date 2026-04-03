#include <stdio.h>
#include "slang.h"

int
main()
{
	/* Initialize the library.  This is always needed. */

	if (!init_SLang()		       /* basic interpreter functions */
			|| !init_SLmath() 	       /* sin, cos, etc... */
#ifdef unix
			|| !init_SLunix()	       /* unix system calls */
#endif
			|| !init_SLfiles()) {
		fprintf(stderr, "Unable to initialize S-Lang.\n");
		exit(-1);
	}
	SLang_Traceback = 1;
	sl_plot_init();
	SLang_load_file ("plot_tst.sl");

	return 0;
}

