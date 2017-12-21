#include "bkpr.h"

#define OPCMP(x)	(strcmp(operation,x) == 0)

bkpr_context	*ctx;

int
main(int argc, char *argv[])
{
	int	r, ch;
	char	*operation;

	if ((ctx = ctx_alloc()) == NULL)
		err(ENOMEM,"cannot allocate context");

	ctx->verbosity = BKPR_VERB_STD;
#ifdef DB_SQLITE
	ctx->dbtype = BKPR_DBTYPE_SQLITE;
#endif

	if ((ctx->err = err_alloc()) == NULL)
		err(ENOMEM,"cannot allocate error structure");

	ctx->cfgfile = strdup(BKPR_CFGFILE_DEFAULT);

	opterr = 0;
	while ((ch = getopt(argc,argv,"f:qdN")) != -1) {
		switch(ch) {
			case 'f':
				free(ctx->cfgfile);
				ctx->cfgfile = strdup(optarg);
				break;
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

	if (cfg_load()) {
		errprint();
		exit(ctx->err->no);
	}

	operation = strdup(argv[0]);

	if (OPCMP("help")) {
		free(operation);
		usage();
		exit(BKPR_GOOD);
	}

	if (db_init()) {
		errprint();
		exit(BKPR_BAD);
	}

	r = BKPR_BAD;

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

	if (r != BKPR_GOOD)
		errprint();

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

