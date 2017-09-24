#include "bkpr.h"

extern bkpr_context_t	*ctx;

bkpr_context_t *
ctx_alloc(void)
{
	bkpr_context_t	*p;

	if ((p = calloc(1,sizeof(bkpr_context_t))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (p);
}

bkpr_err_t *
err_alloc(void)
{
	bkpr_err_t	*p;

	if ((p = calloc(1,sizeof(bkpr_err_t))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (p);
}

guest_disk_t *
disk_alloc(int n)
{
	guest_disk_t	*d;

	if ((d = calloc(n,sizeof(guest_disk_t))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (d);
}

void
disk_free(guest_disk_t *d)
{
	if (d != NULL)
		free(d);
	d = NULL;
}

void
disk_free_all(guest_disk_t *d)
{
	guest_disk_t	*t;

	while (d != NULL) {
		t = d->n;
		disk_free(d);
		d = t;
	}
}

void
disk_list_attach(guest_disk_t *lst, guest_disk_t *new)
{
	guest_disk_t	*tn, *tp;

	tn = lst;
	tp = NULL;
	while (tn != NULL) {
		tp = tn;
		tn = tn->n;
	}
	new->p = tp;
	new->p->n = new;
}

guest_disk_t *
disk_list_remove(guest_disk_t *lst, char *path)
{
	guest_disk_t	*t;

	if ((t = disk_list_find(lst,path)) == NULL)
		return (lst);

	if (t == lst) {			/* first node */
		lst = lst->n;
		lst->p = NULL;
	} else if (t->n == NULL) {	/* last node */
		t->p->n = NULL;
	} else {
		t->p->n = t->n;
		t->n->p = t->p;
	}
	disk_free(t);

	return (lst);
}

guest_disk_t *
disk_list_find(guest_disk_t *lst, char *path)
{
	guest_disk_t	*t;
	int		f = 0;

	t = lst;
	while (t != NULL) {
		if (strcmp(t->path,path) == 0) {
			f = 1;
			break;
		}
		t = t->n;
	}

	if (f)
		return (t);
	else
		return (NULL);
}

void
disk_dump(guest_disk_t *d)
{

	if (d == NULL) {
		printf("disk { }\n");
		return;
	}

	printf("disk {\n");
	printf("\t%-8s %d\n","vmid",d->vmid);
	printf("\t%-8s %d\n","diskid",d->diskid);
	printf("\t%-8s %s\n","type",disk_type_str(d->type));
	printf("\t%-8s %s\n","path",d->path);
	printf("\t%-8s %d\n","root",d->root);
	printf("\t%-8s %d\n","cloned",d->cloned);
	if (d->cloned)
		printf("\t%-8s %s\n","cloneof",d->cloneof);
	printf("\t%-8s %p\n","addr",d);
	printf("}\n");
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

guest_nic_t *
nic_alloc(int c)
{
	guest_nic_t	*n;

	if ((n = calloc(c,sizeof(guest_nic_t))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (NULL);
	}

	return (n);
}

void
nic_free(guest_nic_t *n)
{
	if (n != NULL)
		free(n);
	n = NULL;
}

void
nic_free_all(guest_nic_t *lst)
{
	guest_nic_t *t;

	t = lst;
	while (lst != NULL) {
		t = lst->n;
		nic_free(lst);
		lst = t;
	}
}

void
nic_list_attach(guest_nic_t *lst, guest_nic_t *new)
{
	guest_nic_t	*tn, *tp;

	tn = lst;
	tp = NULL;
	while (tn != NULL) {
		tp = tn;
		tn = tn->n;
	}
	new->p = tp;
	new->p->n = new;
}

guest_nic_t *
nic_list_remove(guest_nic_t *lst, int tap)
{
	guest_nic_t	*t;

	if ((t = nic_list_find(lst,tap)) == NULL)
		return (lst);

	if (t == lst) {			/* first node */
		lst = lst->n;
		if (lst != NULL)
			lst->p = NULL;
	} else if (t->n == NULL) {	/* last node */
		t->p->n = NULL;
	} else {
		t->p->n = t->n;
		t->n->p = t->p;
	}
	nic_free(t);

	return (lst);
}

guest_nic_t *
nic_list_find(guest_nic_t *lst, int tap)
{
	guest_nic_t	*t;
	int		f = 0;

	t = lst;
	while (t != NULL) {
		if (t->tap == tap) {
			f = 1;
			break;
		}
		t = t->n;
	}

	if (f)
		return (t);
	else
		return (NULL);
}

void
nic_dump(guest_nic_t *n)
{
	if (n == NULL) {
		printf("nic { }\n");
		return;
	}

	printf("nic {\n");
	printf("\t%-8s %d\n","nicid",n->nicid);
	printf("\t%-8s %d\n","vmid",n->vmid);
	printf("\t%-8s %d\n","tap",n->tap);
	printf("\t%-8s %d\n","bridge",n->bridge);
	printf("\t%-8s %p\n","addr",n);
	printf("}\n");
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
		return (BKPR_DBTYPE_INVALID);
}

char * /* must free */
db_type_str(bkpr_db_type type)
{
	char	*str;

	switch(type) {
		case BKPR_DBTYPE_INVALID:
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

