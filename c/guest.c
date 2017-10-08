#include "bkpr.h"

extern bkpr_context	*ctx;

guest *
guest_alloc(void)
{
	guest	*g;

	if ((g = calloc(1,sizeof(guest))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (g);
}

void
guest_free(guest *g)
{

	if (g == NULL)
		return;

	if (g->disk != NULL)
		disk_free_all(g->disk);
	if (g->nic != NULL)
		nic_free_all(g->nic);
	free(g);
}

void
guest_dump(guest *g)
{
	guest_disk	*disk;
	guest_nic	*nic;

	if (g == NULL) {
		printf("guest {}\n");
		return;
	}

	disk = g->disk;
	nic = g->nic;

	printf("guest {\n");
	printf("\t%-8s %lu\n","vmid",g->vmid);
	printf("\t%-8s %s\n","name",g->name);
	printf("\t%-8s %d\n","cpu",g->cpu);
	printf("\t%-8s %lu\n","mem",g->mem);
	printf("\t%-8s %s\n","os",guest_os_type_str(g->os));
	printf("\t%-8s %s\n","loader",guest_loader_type_str(g->loader));

	if (disk == NULL)
		printf("\tdisks { }\n");
	else {
		printf("\tdisks {\n");
		do {
			disk_dump_simple(2,disk);
			if (disk)
				disk = disk->next;
		} while (disk != NULL);
		printf("\t}\n");
	}

	if (nic == NULL)
		printf("\tnics { }\n");
	else {
		printf("\tnics {\n");
		do {
			nic_dump_simple(2,nic);
			if (nic)
				nic = nic->next;
		} while (nic != NULL);
		printf("\t}\n");
	}

	printf("\t%-8s %s\n","descr",g->descr);
	//printf("\t%-8s %p\n","addr",g);
	printf("}\n");
}

guest_os
guest_os_type(char *str)
{
	char		*t;
	guest_os	r;

	if ((str == NULL) || (strlen(str) == 0))
		return (BKPR_OS_INVAL);

	t = lc(str);

#define OSCMP(x)	(strcmp(t,x) == 0)

	if (OSCMP("freebsd"))
		r = BKPR_OS_FREEBSD;
	else if (OSCMP("openbsd"))
		r = BKPR_OS_OPENBSD;
	else if (OSCMP("netbsd"))
		r = BKPR_OS_NETBSD;
	else if (OSCMP("linux"))
		r = BKPR_OS_LINUX;
	else if (OSCMP("sun"))
		r = BKPR_OS_SUN;
	else if (OSCMP("windows") || OSCMP("win"))
		r = BKPR_OS_WINDOWS;
	else
		r = BKPR_OS_INVAL;

	free(t);

	return (r);
}

char	*
guest_os_type_str(guest_os os)
{

	switch(os) {
		case BKPR_OS_FREEBSD:	return (BKPR_OS_STR_FREEBSD);
		case BKPR_OS_OPENBSD:	return (BKPR_OS_STR_OPENBSD);
		case BKPR_OS_NETBSD:	return (BKPR_OS_STR_NETBSD);
		case BKPR_OS_LINUX:	return (BKPR_OS_STR_LINUX);
		case BKPR_OS_SUN:	return (BKPR_OS_STR_SUN);
		case BKPR_OS_WINDOWS:	return (BKPR_OS_STR_WINDOWS);
		case BKPR_OS_INVAL:
		default:		return (BKPR_OS_STR_INVAL);
	}
}

guest_loader
guest_loader_type(char *in)
{
	char		*t;
	guest_loader	r;

	if ((in == NULL) || (strlen(in) == 0))
		return (BKPR_LOADER_INVAL);

	t = lc(in);

#define LDCMP(x)	(strcmp(t,x) == 0)

	if (LDCMP("bhyveload"))
		r = BKPR_LOADER_BHYVELOAD;
	else if (LDCMP("grub"))
		r = BKPR_LOADER_GRUB;
	else if (LDCMP("uefi"))
		r = BKPR_LOADER_UEFI;
	else if (LDCMP("uefi-csm"))
		r = BKPR_LOADER_UEFI_CSM;
	else
		r = BKPR_LOADER_INVAL;

	free(t);
	return (r);
}

char *
guest_loader_type_str(guest_loader ld)
{

	switch(ld) {
		case BKPR_LOADER_BHYVELOAD:	return (BKPR_LOADER_STR_BHYVELOAD);
		case BKPR_LOADER_GRUB:		return (BKPR_LOADER_STR_GRUB);
		case BKPR_LOADER_UEFI:		return (BKPR_LOADER_STR_UEFI);
		case BKPR_LOADER_UEFI_CSM:	return (BKPR_LOADER_STR_UEFI_CSM);
		case BKPR_LOADER_INVAL:
		default:			return (BKPR_LOADER_STR_INVAL);
	}
}

