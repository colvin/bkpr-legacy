#ifndef BKPR_H
#define BKPR_H

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BKPR_VERSION	"0.0"

#define BKPR_GOOD	EXIT_SUCCESS
#define BKPR_BAD	EXIT_FAILURE

#include "bkpr_types.h"
#include "bkpr_db.h"

void		 usage(void);

int		 create(int, char **);
int		 test(void);	/* beware */

bkpr_context	*ctx_alloc(void);
bkpr_err	*err_alloc(void);

int		 errset(int, char *, ...);
int		 errget(void);
char		*errstr(void);
void		 errprint(void);
void		 complain(char *, ...);

#define UNIMP(x)	complain("unimplemented: %s",x)
#define DODBG()		(ctx->verbosity == BKPR_VERB_DEBUG)

#define GUEST_DEFAULT_CPU	1
#define GUEST_DEFAULT_MEM	1024
#define GUEST_DEFAULT_OS	"freebsd"
#define GUEST_DEFAULT_LOADER	"bhyveload"

guest		*guest_alloc(void);
void		 guest_free(guest *);
void		 guest_dump(guest *);
guest_os	 guest_os_type(char *);
char		*guest_os_type_str(guest_os);
guest_loader	 guest_loader_type(char *);
char		*guest_loader_type_str(guest_loader);

guest_disk	*disk_alloc(void);
void		 disk_free(guest_disk *);
void		 disk_free_all(guest_disk *);
guest_disk	*disk_spec(char *);
void		 disk_list_attach(guest_disk *, guest_disk *);
guest_disk	*disk_list_remove(guest_disk *, char *);
guest_disk	*disk_list_find(guest_disk *, char *);
void		 disk_dump(int, guest_disk *);
void		 disk_dump_simple(int, guest_disk *);
char		*disk_type_str(guest_disk_type);
guest_disk_type	 disk_type(char *);

#define BKPR_DEFAULT_BRIDGE	0

guest_nic	*nic_alloc(void);
void		 nic_free(guest_nic *);
void		 nic_free_all(guest_nic *);
guest_nic	*nic_spec(char *);
guest_nic	*nic_spec_auto(void);
int		 nic_next_tapid(void);
void		 nic_list_attach(guest_nic *, guest_nic *);
guest_nic	*nic_list_remove(guest_nic *, int);
guest_nic	*nic_list_find(guest_nic *, int);
void		 nic_dump(int, guest_nic *);
void		 nic_dump_simple(int, guest_nic *);

int		 chomp(char *);
char		*lc(char *); /* must free */
int		str_isnumber(char *);

#endif /* BKPR_H */
