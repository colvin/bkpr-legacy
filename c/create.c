#include "bkpr.h"

int	create_disk_add(guest *, char *, int);

int
create(int argc, char **argv)
{
	int ch, rflag;
	guest	*g;

	if ((g = guest_alloc()) == NULL)
		return (ENOMEM);

	rflag = 0;
	while ((ch = getopt(argc,argv,"n:c:m:o:l:r:d:D:")) != -1) {
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
			case 'r':
				if (rflag) {
					errset(EINVAL,"must have exactly one root disk");
					guest_free(g);
					return (EXIT_FAILURE);
				}
				if (create_disk_add(g,optarg,1) != EXIT_SUCCESS) {
					guest_free(g);
					return (EXIT_FAILURE);
				}
				rflag = 1;
				break;
			case 'd':
				if (create_disk_add(g,optarg,0) != EXIT_SUCCESS) {
					guest_free(g);
					return (EXIT_FAILURE);
				}
				break;
			case 'D':
				snprintf(g->descr,sizeof(g->descr),"%s",optarg);
				break;
			case '?':
			default:
				printf("invalid option: -%c\n",optopt);
				guest_free(g);
				return (EINVAL);
		}
	}
	argv += optind;
	argc -= optind;

	if (rflag == 0) {
		errset(EINVAL,"must have exactly one root disk");
		guest_free(g);
		return (EXIT_FAILURE);
	}

	guest_dump(g);

	guest_free(g);
	return (EXIT_SUCCESS);
}

int
create_disk_add(guest *g, char *spec, int root)
{
	guest_disk	*d;

	if (g == NULL) {
		errset(EINVAL,"create_disk_add(): guest is NULL");
		return (EXIT_FAILURE);
	}
	if (spec == NULL) {
		errset(EINVAL,"create_disk_add(): disk spec is NULL");
		return (EXIT_FAILURE);
	}

	if ((d = disk_spec(spec)) == NULL)
		return (EXIT_FAILURE);

	if (root)
		d->root = 1;

	disk_dump(0,d);

	if (g->disk == NULL)
		g->disk = d;
	else
		disk_list_attach(g->disk,d);

	return (EXIT_SUCCESS);
}

