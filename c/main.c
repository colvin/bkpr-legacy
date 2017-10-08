#include "bkpr.h"

#define OPCMP(x)	(strcmp(operation,x) == 0)

bkpr_context	*ctx;

int
main(int argc, char *argv[])
{
	int	r, ch;
	char	*operation;

	if ((ctx = ctx_alloc()) == NULL) {
		errprint();
		exit(ENOMEM);
	}

	ctx->verbosity = BKPR_VERB_STD;
#ifdef DB_SQLITE
	ctx->dbtype = BKPR_DBTYPE_SQLITE;
#endif

	if ((ctx->err = err_alloc()) == NULL) {
		errprint();
		exit(ENOMEM);
	}

	opterr = 0;
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
				exit(EINVAL);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage();
		exit(EINVAL);
	}

	if ((operation = strdup(argv[0])) == NULL)
		err(ENOMEM,"out of memory");

	if (OPCMP("help")) {
		free(operation);
		usage();
		exit(EXIT_SUCCESS);
	}

	if (db_init()) {
		errprint();
		exit(EXIT_FAILURE);
	}

	r = EXIT_FAILURE;

	if (OPCMP("list"))
		UNIMP("list");
	else if (OPCMP("status"))
		UNIMP("status");
	else if (OPCMP("create"))
		r = create(argc, argv);
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
		complain("unknown operation: %s",operation);
		free(operation);
		db_disconnect();
		usage();
		exit(EINVAL);
	}

	free(operation);
	db_disconnect();

	return (r);
}

void
usage(void)
{

	if (ctx->verbosity == BKPR_VERB_QUIET)
		return;

	fprintf(stderr,"\n");
	fprintf(stderr,"bkpr %s\n",BKPR_VERSION);
	fprintf(stderr,"usage: bkpr [-qdN] <operation> [operation-flags]\n");
	fprintf(stderr,"   -q: quiet verbosity\n");
	fprintf(stderr,"   -d: debugging verbosity\n");
	fprintf(stderr,"   -N: no-operation mode\n");
	fprintf(stderr,"\n");
}

