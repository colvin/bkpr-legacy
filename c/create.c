#include "bkpr.h"

extern bkpr_context	*ctx;

int	create_disk_add(guest *, char *, int);
int	create_nic_add(guest *, char *);

int
create(int argc, char **argv)
{
	int ch, nflag, rflag, oflag, lflag;
	guest	*g;

	if ((g = guest_alloc()) == NULL)
		return (ENOMEM);

	nflag = rflag = oflag = lflag = 0;
	optreset = optind = 1;
	while ((ch = getopt(argc,argv,"n:c:m:o:l:r:d:N:D:")) != -1) {
		switch(ch) {
			case 'n':
				snprintf(g->name,sizeof(g->name),"%s",optarg);
				nflag = 1;
				break;
			case 'c':
				g->cpu = strtol(optarg,NULL,0);
				break;
			case 'm':
				g->mem = strtol(optarg,NULL,0);
				break;
			case 'o':
				g->os = guest_os_type(optarg);
				oflag = 1;
				break;
			case 'l':
				g->loader = guest_loader_type(optarg);
				lflag = 1;
				break;
			case 'r':
				if (rflag) {
					errset(EINVAL,"must have exactly one root disk");
					guest_free(g);
					return (BKPR_BAD);
				}
				if (create_disk_add(g,optarg,1) != BKPR_GOOD) {
					guest_free(g);
					return (BKPR_BAD);
				}
				rflag = 1;
				break;
			case 'd':
				if (create_disk_add(g,optarg,0) != BKPR_GOOD) {
					guest_free(g);
					return (BKPR_BAD);
				}
				break;
			case 'N':
				if (create_nic_add(g,optarg) != BKPR_GOOD) {
					guest_free(g);
					return (BKPR_BAD);
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

	if (!nflag) {
		errset(EINVAL,"must have a guest name");
		guest_free(g);
		return (EINVAL);
	}

	if (!rflag) {
		errset(EINVAL,"must have exactly one root disk");
		guest_free(g);
		return (EINVAL);
	}

	if (g->cpu == 0)
		g->cpu = GUEST_DEFAULT_CPU;
	if (g->mem == 0)
		g->mem = GUEST_DEFAULT_MEM;
	if (!oflag)
		g->os = BKPR_OS_FREEBSD;
	if (!lflag) {
		if (!oflag)
			g->loader = BKPR_LOADER_BHYVELOAD;
		else {
			errset(EINVAL,"must give a loader if not using os default");
			guest_free(g);
			return (EINVAL);
		}
	}

	guest_dump(g);

	guest_free(g);
	return (BKPR_GOOD);
}

int
create_disk_add(guest *g, char *spec, int root)
{
	guest_disk	*d;

	if (g == NULL) {
		errset(EINVAL,"create_disk_add(): guest is NULL");
		return (BKPR_BAD);
	}
	if (spec == NULL) {
		errset(EINVAL,"create_disk_add(): disk spec is NULL");
		return (BKPR_BAD);
	}

	if ((d = disk_spec(spec)) == NULL)
		return (BKPR_BAD);

	if (root)
		d->root = 1;

	if (DODBG())
		disk_dump(0,d);

	if (g->disk == NULL)
		g->disk = d;
	else
		disk_list_attach(g->disk,d);

	return (BKPR_GOOD);
}

int
create_nic_add(guest *g, char *spec)
{
	guest_nic	*n;

	if (g == NULL) {
		errset(EINVAL,"create_nic_add(): guest is NULL");
		return (BKPR_BAD);
	}
	if (spec == NULL) {
		errset(EINVAL,"create_nic_add(): disk spec is NULL");
		return (BKPR_BAD);
	}

	if ((n = nic_spec(spec)) == NULL)
		return (BKPR_BAD);

	if (DODBG())
		nic_dump(0,n);

	if (g->nic == NULL)
		g->nic = n;
	else
		nic_list_attach(g->nic,n);

	return (BKPR_GOOD);
}

