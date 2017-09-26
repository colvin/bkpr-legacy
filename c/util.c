#include "bkpr.h"

extern bkpr_context	*ctx;

bkpr_context *
ctx_alloc(void)
{
	bkpr_context	*p;

	if ((p = calloc(1,sizeof(bkpr_context))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (p);
}

bkpr_err *
err_alloc(void)
{
	bkpr_err	*p;

	if ((p = calloc(1,sizeof(bkpr_err))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (p);
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

	do {
		disk_dump(1,disk);
		if (disk)
			disk = disk->next;
	} while (disk != NULL);

	do {
		nic_dump(1,nic);
		if (nic)
			nic = nic->next;
	} while (nic != NULL);

	printf("\t%-8s %s\n","descr",g->descr);
	printf("\t%-8s %p\n","addr",g);
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

guest_disk *
disk_alloc(int n)
{
	guest_disk	*d;

	if ((d = calloc(n,sizeof(guest_disk))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (d);
}

void
disk_free(guest_disk *d)
{
	if (d != NULL)
		free(d);
	d = NULL;
}

void
disk_free_all(guest_disk *d)
{
	guest_disk	*t;

	while (d != NULL) {
		t = d->next;
		disk_free(d);
		d = t;
	}
}

void
disk_list_attach(guest_disk *lst, guest_disk *new)
{
	guest_disk	*tn, *tp;

	tn = lst;
	tp = NULL;
	while (tn != NULL) {
		tp = tn;
		tn = tn->next;
	}
	new->prev = tp;
	new->prev->next = new;
}

guest_disk *
disk_list_remove(guest_disk *lst, char *path)
{
	guest_disk	*t;

	if ((t = disk_list_find(lst,path)) == NULL)
		return (lst);

	if (t == lst) {			/* first node */
		lst = lst->next;
		lst->prev = NULL;
	} else if (t->next == NULL) {	/* last node */
		t->prev->next = NULL;
	} else {
		t->prev->next = t->next;
		t->next->prev = t->prev;
	}
	disk_free(t);

	return (lst);
}

guest_disk *
disk_list_find(guest_disk *lst, char *path)
{
	guest_disk	*t;
	int		f = 0;

	t = lst;
	while (t != NULL) {
		if (strcmp(t->path,path) == 0) {
			f = 1;
			break;
		}
		t = t->next;
	}

	if (f)
		return (t);
	else
		return (NULL);
}

void
disk_dump(int lvl, guest_disk *d)
{
	char	ind[BKPR_SZ_MAXINDENT];

	memset(ind,'\0',sizeof(ind));
	for (int i = 0; ((i < lvl) && (i < sizeof(ind))); i++)
		ind[i] = '\t';

	if (d == NULL) {
		printf("%sdisk { }\n",ind);
		return;
	}

	printf("%sdisk {\n",ind);
	printf("%s\t%-8s %d\n",ind,"vmid",d->vmid);
	printf("%s\t%-8s %d\n",ind,"diskid",d->diskid);
	printf("%s\t%-8s %s\n",ind,"type",disk_type_str(d->type));
	printf("%s\t%-8s %s\n",ind,"path",d->path);
	printf("%s\t%-8s %d\n",ind,"root",d->root);
	printf("%s\t%-8s %d\n",ind,"cloned",d->cloned);
	if (d->cloned)
		printf("%s\t%-8s %s\n",ind,"cloneof",d->cloneof);
	printf("%s\t%-8s %p\n",ind,"addr",d);
	printf("%s}\n",ind);
}

char *
disk_type_str(guest_disk_type dt)
{

	switch(dt) {
		case DISK_TYPE_INVAL:
			return (DISK_TYPE_STR_INVAL);
		case DISK_TYPE_FILE:
			return (DISK_TYPE_STR_FILE);
		case DISK_TYPE_ZVOL:
			return (DISK_TYPE_STR_ZVOL);
		default:
			return (DISK_TYPE_STR_INVAL);
	}
}

guest_disk_type
disk_type(char *str)
{

	if (strcmp(str,"file") == 0)
		return (DISK_TYPE_FILE);
	else if (strcmp(str,"zvol") == 0)
		return (DISK_TYPE_ZVOL);
	else
		return (DISK_TYPE_INVAL);
}

/* nics */

guest_nic *
nic_alloc(int c)
{
	guest_nic	*n;

	if ((n = calloc(c,sizeof(guest_nic))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (n);
}

void
nic_free(guest_nic *n)
{
	if (n != NULL)
		free(n);
	n = NULL;
}

void
nic_free_all(guest_nic *lst)
{
	guest_nic *t;

	t = lst;
	while (lst != NULL) {
		t = lst->next;
		nic_free(lst);
		lst = t;
	}
}

void
nic_list_attach(guest_nic *lst, guest_nic *new)
{
	guest_nic	*tn, *tp;

	tn = lst;
	tp = NULL;
	while (tn != NULL) {
		tp = tn;
		tn = tn->next;
	}
	new->prev = tp;
	new->prev->next = new;
}

guest_nic *
nic_list_remove(guest_nic *lst, int tap)
{
	guest_nic	*t;

	if ((t = nic_list_find(lst,tap)) == NULL)
		return (lst);

	if (t == lst) {			/* first node */
		lst = lst->next;
		if (lst != NULL)
			lst->prev = NULL;
	} else if (t->next == NULL) {	/* last node */
		t->prev->next = NULL;
	} else {
		t->prev->next = t->next;
		t->next->prev = t->prev;
	}
	nic_free(t);

	return (lst);
}

guest_nic *
nic_list_find(guest_nic *lst, int tap)
{
	guest_nic	*t;
	int		f = 0;

	t = lst;
	while (t != NULL) {
		if (t->tap == tap) {
			f = 1;
			break;
		}
		t = t->next;
	}

	if (f)
		return (t);
	else
		return (NULL);
}

void
nic_dump(int lvl, guest_nic *n)
{
	char	ind[BKPR_SZ_MAXINDENT];

	memset(ind,'\0',sizeof(ind));
	for (int i = 0; ((i < lvl) && (i < sizeof(ind))); i++)
		ind[i] = '\t';

	if (n == NULL) {
		printf("%snic { }\n",ind);
		return;
	}

	printf("%snic {\n",ind);
	printf("%s\t%-8s %d\n",ind,"nicid",n->nicid);
	printf("%s\t%-8s %d\n",ind,"vmid",n->vmid);
	printf("%s\t%-8s %d\n",ind,"tap",n->tap);
	printf("%s\t%-8s %d\n",ind,"bridge",n->bridge);
	printf("%s\t%-8s %p\n",ind,"addr",n);
	printf("%s}\n",ind);
}

bkpr_db_type
db_type(char *str)
{

#define DBTYPECMP(x)	(strcmp(str,x) == 0)

	if (DBTYPECMP("sqlite"))
		return (BKPR_DBTYPE_SQLITE);
	else if (DBTYPECMP("mysql"))
		return (BKPR_DBTYPE_MYSQL);
	else
		return (BKPR_DBTYPE_INVAL);
}

char * /* must free */
db_type_str(bkpr_db_type type)
{
	char	*str;

	switch(type) {
		case BKPR_DBTYPE_INVAL:
			str = strdup("invalid");
			break;
		case BKPR_DBTYPE_SQLITE:
			str = strdup("sqlite");
			break;
		case BKPR_DBTYPE_MYSQL:
			str = strdup("mysql");
			break;
		default:
			str = strdup("invalid");
			break;
	}

	return (str);
}

int
chomp(char *str)
{
	int	c = 0;
	size_t	l = 0;

	if (str == NULL)
		return (0);

	l = strlen(str);
	if (l == 0)
		return (0);

	while (str[l-1] == '\n') {
		str[l-1] = '\0';
		l--;
		c++;
	}

	return (c);
}

char *	/* must free */
lc(char *in)
{
	int	c;
	char	*out;

	if ((in == NULL) || (strlen(in) == 0))
		return (NULL);

	out = strdup(in);

	for (int i = 0; i < strlen(in); i++)
		out[i] = tolower(out[i]);

	return (out);
}

