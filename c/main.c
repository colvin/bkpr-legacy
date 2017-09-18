#include "bkpr.h"

#define OPCMP(x)	(strcmp(operation,x) == 0)

bkpr_context_t	*ctx;

int
main(int argc, char *argv[])
{
	int	r, ch;
	char	*operation;

	if ((ctx = ctx_alloc()) == NULL)
		err(ENOMEM,"out of memory");

	if ((ctx->err = err_alloc()) == NULL)
		err(ENOMEM,"out of memory");

	while ((ch = getopt(argc,argv,"qdN")) != -1) {
		switch(ch) {
			case 'q':
				ctx->verbosity = BKPR_VERB_QUIET;
				break;
			case 'd':
				ctx->verbosity = BKPR_VERB_DEBUG;
				break;
			case 'N':
				ctx->noop = 1;
				break;
			default:
				usage();
				exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage();
		exit(EXIT_FAILURE);
	}

	if ((operation = strdup(argv[0])) == NULL)
		err(ENOMEM,"out of memory");

	r = EXIT_FAILURE;

	if (OPCMP("list"))
		UNIMP("list");
	else if (OPCMP("status"))
		UNIMP("status");
	else if (OPCMP("create"))
		UNIMP("create");
	else if (OPCMP("run"))
		UNIMP("run");
	else if (OPCMP("kill"))
		UNIMP("kill");
	else if (OPCMP("destroy"))
		UNIMP("destroy");
	else if (OPCMP("update"))
		UNIMP("update");
	else if (OPCMP("test"))
		r = test();
	else {
		fprintf(stderr,"unknown operation: %s\n",operation);
		usage();
		exit(EXIT_FAILURE);
	}
	free(operation);

	return r;
}

void
usage(void)
{

	fprintf(stderr,"\n");
	fprintf(stderr,"bkpr %s\n",BKPR_VERSION);
	fprintf(stderr,"usage: bkpr [-qdN] <operation> [operation-flags]\n");
	fprintf(stderr,"   -q: quiet verbosity\n");
	fprintf(stderr,"   -d: debugging verbosity\n");
	fprintf(stderr,"   -N: no-operation mode\n");
	fprintf(stderr,"\n");
}

