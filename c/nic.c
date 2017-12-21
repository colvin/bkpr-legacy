#include "bkpr.h"

extern bkpr_context	*ctx;

guest_nic *
nic_alloc(void)
{
	guest_nic	*n;

	if ((n = calloc(1,sizeof(guest_nic))) == NULL) {
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

guest_nic *
nic_spec(char *spec)
{
	char		*br, *tp;
	guest_nic	*n;

	if ((spec == NULL) || (strcmp(spec,"auto") == 0) || (strcmp(spec,"::") == 0))
		return nic_spec_auto();

	if (strstr(spec,"::") == NULL) {
		errset(EINVAL,"invalid spec string");
		return (NULL);
	}

	if ((n = nic_alloc()) == NULL)
		return (NULL);

	br = strtok(spec,"::");
	if (str_isnumber(br))
		n->bridge = strtol(br,NULL,0);
	else {
		errset(EINVAL,"bridgeid is not a number");
		nic_free(n);
		return (NULL);
	}

	tp = strtok(NULL,"::");
	if (strcmp(tp,"auto") == 0)
		n->tap = nic_next_tapid();
	else {
		if (str_isnumber(tp))
			n->tap = strtol(tp,NULL,0);
		else {
			errset(EINVAL,"tapid is not a number");
			nic_free(n);
			return (NULL);
		}
	}

	return (n);
}

guest_nic *
nic_spec_auto(void)
{
	guest_nic	*n;

	if ((n = nic_alloc()) == NULL)
		return (NULL);

	n->bridge = BKPR_DEFAULT_BRIDGE;
	n->tap = nic_next_tapid();

	return (n);
}

int
nic_next_tapid(void)
{
	int	r;

	/* TODO */
	srand(getpid());
	r = rand();

	return (r);
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
	printf("%s\t%-8s %d\n",ind,"nic_id",n->nic_id);
	printf("%s\t%-8s %d\n",ind,"guest_id",n->guest_id);
	printf("%s\t%-8s %d\n",ind,"tap",n->tap);
	printf("%s\t%-8s %d\n",ind,"bridge",n->bridge);
	//printf("%s\t%-8s %p\n",ind,"addr",n);
	printf("%s}\n",ind);
}

void
nic_dump_simple(int lvl, guest_nic *n)
{
	char	ind[BKPR_SZ_MAXINDENT];

	if (n == NULL)
		return;

	memset(ind,'\0',sizeof(ind));
	for (int i = 0; ((i < lvl) && (i < sizeof(ind))); i++)
		ind[i] = '\t';

	printf("%s bridge%d::tap%d\n",ind,n->bridge,n->tap);
}

