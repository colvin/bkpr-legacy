#ifndef BKPR_H
#define BKPR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>

#define BKPR_VERSION	"0.0"

#include "bkpr_types.h"
#include "bkpr_db.h"

#define UNIMP(x)	fprintf(stderr,"unimplemented: %s\n",x)

void		usage(void);

int		test(void);	/* beware */

bkpr_context_t	*ctx_alloc(void);
bkpr_err_t	*err_alloc(void);

guest_disk_t	*disk_alloc(int);
void		disk_free(guest_disk_t *);
void		disk_free_all(guest_disk_t *);
void		disk_list_attach(guest_disk_t *, guest_disk_t *);
guest_disk_t	*disk_list_remove(guest_disk_t *, char *);
guest_disk_t	*disk_list_find(guest_disk_t *, char *);
void		disk_dump(guest_disk_t *);
char		*disk_type_str(guest_disk_type);
guest_disk_type	disk_type(char *);

guest_nic_t	*nic_alloc(int);
void		nic_free(guest_nic_t *);
void		nic_free_all(guest_nic_t *);
void		nic_list_attach(guest_nic_t *, guest_nic_t *);
guest_nic_t	*nic_list_remove(guest_nic_t *, int);
guest_nic_t	*nic_list_find(guest_nic_t *, int);
void		nic_dump(guest_nic_t *);

#endif /* BKPR_H */

