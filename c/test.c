#include "bkpr.h"

extern bkpr_context	*ctx;

int
test(void)
{
	guest		*g;
	guest_nic	*n1;
	guest_disk	*d1, *d2;

	if ((g = calloc(1,sizeof(guest))) == NULL)
		return (ENOMEM);

	g->guest_id = 1;
	snprintf(g->name,sizeof(g->name),"%s","foomatic");
	g->cpu = 4;
	g->mem = 6144;
	g->os = guest_os_type("FreeBSD");
	g->loader = guest_loader_type("bhyveload");
	snprintf(g->descr,sizeof(g->descr),"%s","A sample FreeBSD guest");

	n1 = calloc(1,sizeof(guest_nic));
	n1->nic_id = 1;
	n1->guest_id = 1;
	n1->bridge = 0;
	n1->tap = 0;
	g->nic = n1;

	d1 = disk_alloc();
	d1->disk_id = 1;
	d1->guest_id = 1;
	d1->type = DISK_TYPE_ZVOL;
	snprintf(d1->path,sizeof(d1->path),"%s","tank/vol/foo");
	d1->root = 1;

	g->disk = d1;

	d2 = disk_alloc();
	d2->disk_id = 2;
	d2->guest_id = 1;
	d2->type = DISK_TYPE_FILE;
	snprintf(d2->path,sizeof(d2->path),"%s","/bkpr/foomatic/disk2.img");
	d2->root = 0;
	disk_list_attach(g->disk,d2);

	guest_dump(g);

	disk_free_all(g->disk);
	nic_free_all(g->nic);
	free(g);

	return 0;
}

