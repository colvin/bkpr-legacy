#include "bkpr.h"

extern bkpr_err_t	*e;

int
test(void)
{

	guest_disk_t	*sp, *disk, *disk1, *disk2, *disk3;
	if ((disk1 = disk_alloc(1)) == NULL)
		err(ENOMEM,"out of memory");
	disk1->diskid = 1;
	disk1->vmid = 1;
	disk1->type = DISK_TYPE_FILE;
	snprintf(disk1->path,BKPR_SZ_DISK_PATH,"%s","/tmp/foo.img");
	disk1->cloned = 0;
	/*
	disk1->tap = 0;
	disk1->bridge = 0;
	*/

	sp = disk = disk1;

	if ((disk2 = disk_alloc(1)) == NULL)
		err(ENOMEM,"out of memory");
	disk2->diskid = 2;
	disk2->vmid = 1;
	disk2->type = DISK_TYPE_ZVOL;
	snprintf(disk2->path,BKPR_SZ_DISK_PATH,"%s","zstore/vol/bkpr/bar");
	disk2->cloned = 0;
	/*
	disk2->tap = 1;
	disk2->bridge = 0;
	*/

	disk_list_attach(disk,disk2);

	if ((disk3 = disk_alloc(1)) == NULL)
		err(ENOMEM,"out of memory");
	disk3->diskid = 3;
	disk3->vmid = 1;
	snprintf(disk3->path,BKPR_SZ_DISK_PATH,"%s","/tmp/zorp.img");
	disk3->cloned = 1;
	snprintf(disk3->cloneof,BKPR_SZ_DISK_PATH,"%s","/tmp/foo.img");
	/*
	disk3->tap = 2;
	disk3->bridge = 1;
	*/

	disk_list_attach(disk,disk3);

	while (disk != NULL) {
		disk_dump(disk);
		disk = disk->n;
	}

	disk = sp;
	//sp = disk_list_remove(sp,0);
	//sp = disk_list_remove(sp,1);
	//sp = disk_list_remove(sp,2);
	sp = disk_list_remove(sp,"zstore/vol/bkpr/bar");
	//sp = disk_list_remove(sp,"/tmp/foo.img");
	disk = sp;

	printf("--\n");
	while (disk != NULL) {
		disk_dump(disk);
		disk = disk->n;
	}

	disk_free_all(sp);

	return 0;
}

