#include "bkpr.h"

extern bkpr_context	*ctx;

guest_disk *
disk_alloc(void)
{
	guest_disk	*d;

	if ((d = calloc(1,sizeof(guest_disk))) == NULL) {
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

guest_disk *
disk_spec(char *spec)
{
	guest_disk	*d;
	char		*type, *path, *size;

	if (spec == NULL) {
		errset(EINVAL,"disk_spec(): no spec provided");
		return (NULL);
	}

	if (strstr(spec,"::") == NULL) {
		errset(EINVAL,"invalid disk spec");
		return NULL;
	}

	if ((d = disk_alloc()) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	type = strtok(spec,"::");
	if ((d->type = disk_type(type)) == DISK_TYPE_INVAL) {
		errset(EINVAL,"invalid disk type: %s",type);
		disk_free(d);
		return (NULL);
	}

	path = strtok(NULL,"::");
	snprintf(d->path,sizeof(d->path),"%s",path);

	size = strtok(NULL,"::");
	if (size != NULL) {
		if (str_isnumber(size))
			d->size = strtol(size,NULL,0);
		else {
			errset(EINVAL,"disk size is not a number");
			disk_free(d);
			return (NULL);
		}
	}

	return (d);
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
	if (d->size)
		printf("%s\t%-8s %d GiB\n",ind,"size",d->size);
	printf("%s\t%-8s %d\n",ind,"root",d->root);
	printf("%s\t%-8s %d\n",ind,"cloned",d->cloned);
	if (d->cloned)
		printf("%s\t%-8s %s\n",ind,"cloneof",d->cloneof);
	//printf("%s\t%-8s %p\n",ind,"addr",d);
	printf("%s}\n",ind);
}

void
disk_dump_simple(int lvl, guest_disk *d)
{
	char	ind[BKPR_SZ_MAXINDENT];

	if (d == NULL)
		return;

	memset(ind,'\0',sizeof(ind));
	for (int i = 0; ((i < lvl) && (i < sizeof(ind))); i++)
		ind[i] = '\t';

	if (d->root)
		printf("%s %s::%s (root)\n",ind,disk_type_str(d->type),d->path);
	else
		printf("%s %s::%s\n",ind,disk_type_str(d->type),d->path);
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

