#include "bkpr.h"

bkpr_err_t	*e;

int
main(int argc, char *argv[])
{

	if ((e = err_alloc()) == NULL)
		err(ENOMEM,"out of memory");

	test();

	return 0;
}

void
usage(void)
{

	fprintf(stderr,"%s %s\n",PROGRAM,VERSION);
	fprintf(stderr,"usage: %s\n",PROGRAM);
}

