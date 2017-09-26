#include "bkpr.h"

extern bkpr_context	*ctx;

int
test(void)
{
	guest		*g;
	guest_nic	*n1;
	guest_disk	*d1;

	if ((g = calloc(1,sizeof(guest))) == NULL)
		return (ENOMEM);

	g->vmid = 1;
	snprintf(g->name,sizeof(g->name),"%s","foomatic");
	g->cpu = 4;
	g->mem = 6144;
	g->os = guest_os_type("FreeBSD");
	g->loader = guest_loader_type("bhyveload");
	snprintf(g->descr,sizeof(g->descr),"%s","A sample FreeBSD guest");

	n1 = calloc(1,sizeof(guest_nic));
	n1->nicid = 1;
	n1->vmid = 1;
	n1->bridge = 0;
	n1->tap = 0;
	g->nic = n1;

	d1 = calloc(1,sizeof(guest_disk));
	d1->diskid = 1;
	d1->vmid = 1;
	d1->type = DISK_TYPE_ZVOL;
	snprintf(d1->path,sizeof(d1->path),"%s","tank/vol/foo");
	d1->root = 1;
	g->disk = d1;

	guest_dump(g);

	disk_free(d1);
	nic_free(n1);
	free(g);

	return 0;
}

