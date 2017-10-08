#include "bkpr.h"

int
create(int argc, char **argv)
{
	int ch;
	guest	*g;

	if ((g = guest_alloc()) == NULL)
		return (ENOMEM);

	while ((ch = getopt(argc,argv,"n:c:m:o:l:D:")) != -1) {
		switch(ch) {
			case 'n':
				snprintf(g->name,sizeof(g->name),"%s",optarg);
				break;
			case 'c':
				g->cpu = strtol(optarg,NULL,0);
				break;
			case 'm':
				g->mem = strtol(optarg,NULL,0);
				break;
			case 'o':
				g->os = guest_os_type(optarg);
				break;
			case 'l':
				g->loader = guest_loader_type(optarg);
				break;
			case 'D':
				snprintf(g->descr,sizeof(g->descr),"%s",optarg);
				break;
			case '?':
			default:
				printf("invalid option: -%c\n",optopt);
				return (EINVAL);
		}
	}

	guest_dump(g);

	return (EXIT_SUCCESS);
}

