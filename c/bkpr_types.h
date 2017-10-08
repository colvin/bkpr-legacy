#ifndef BKPR_TYPES_H
#define BKPR_TYPES_H

#define BKPR_SZ_ERRSTR		BUFSIZ

typedef enum bkpr_verbosity {
	BKPR_VERB_QUIET,
	BKPR_VERB_STD,
	BKPR_VERB_DEBUG
} bkpr_verbosity;

typedef struct bkpr_err {
	int	no;
	char	str[BKPR_SZ_ERRSTR];
} bkpr_err;

typedef enum bkpr_db_type {
	BKPR_DBTYPE_INVAL,
	BKPR_DBTYPE_SQLITE,
	BKPR_DBTYPE_MYSQL
} bkpr_db_type;

struct bkpr_db;	/* forward declaration */

typedef struct bkpr_context {
	bkpr_verbosity		verbosity;
	bool			noop;
	bkpr_err		*err;
	bkpr_db_type		dbtype;
	struct bkpr_db		*db;
} bkpr_context;

#define BKPR_SZ_GUEST_NAME	256
#define BKPR_SZ_GUEST_OS	16
#define BKPR_SZ_GUEST_LOADER	16
#define BKPR_SZ_GUEST_GRUBMAP	PATH_MAX
#define BKPR_SZ_GUEST_GRUBCMD	PATH_MAX
#define BKPR_SZ_GUEST_DESCR	256
#define BKPR_SZ_DISK_PATH	PATH_MAX
#define BKPR_SZ_MAXINDENT	8

typedef enum guest_disk_type {
	DISK_TYPE_INVAL,
	DISK_TYPE_FILE,
	DISK_TYPE_ZVOL
} guest_disk_type;

#define DISK_TYPE_STR_INVAL	"*invalid*"
#define DISK_TYPE_STR_FILE	"file"
#define DISK_TYPE_STR_ZVOL	"zvol"

typedef struct guest_disk {
	int			diskid;
	int			vmid;
	guest_disk_type		type;
	char			path[BKPR_SZ_DISK_PATH];
	int			root;
	int			cloned;
	char			cloneof[BKPR_SZ_DISK_PATH];
	int			size;	/* only used when creating a disk */
	struct guest_disk	*prev;
	struct guest_disk	*next;
} guest_disk;

typedef struct guest_nic {
	int			nicid;
	int			vmid;
	int			tap;
	int			bridge;
	struct guest_nic	*prev;
	struct guest_nic	*next;
} guest_nic;

typedef enum guest_os {
	BKPR_OS_INVAL,
	BKPR_OS_FREEBSD,
	BKPR_OS_OPENBSD,
	BKPR_OS_NETBSD,
	BKPR_OS_LINUX,
	BKPR_OS_SUN,
	BKPR_OS_WINDOWS
} guest_os;

#define BKPR_OS_STR_INVAL	"*invalid*"
#define BKPR_OS_STR_FREEBSD	"freebsd"
#define BKPR_OS_STR_OPENBSD	"openbsd"
#define BKPR_OS_STR_NETBSD	"netbsd"
#define BKPR_OS_STR_LINUX	"linux"
#define BKPR_OS_STR_SUN		"sun"
#define BKPR_OS_STR_WINDOWS	"windows"

typedef enum guest_loader {
	BKPR_LOADER_INVAL,
	BKPR_LOADER_BHYVELOAD,
	BKPR_LOADER_GRUB,
	BKPR_LOADER_UEFI,
	BKPR_LOADER_UEFI_CSM
} guest_loader;


#define BKPR_LOADER_STR_INVAL		"*invalid*"
#define	BKPR_LOADER_STR_BHYVELOAD	"bhyveload"
#define	BKPR_LOADER_STR_GRUB		"grub"
#define	BKPR_LOADER_STR_UEFI		"uefi"
#define	BKPR_LOADER_STR_UEFI_CSM	"uefi-csm"

typedef struct grub_def {
	char	map[BKPR_SZ_GUEST_GRUBMAP];
	char	cmd[BKPR_SZ_GUEST_GRUBCMD];
} grub_def;

typedef struct guest {
	unsigned long	vmid;
	char		name[BKPR_SZ_GUEST_NAME];
	int		cpu;
	unsigned long	mem;
	guest_os	os;
	guest_loader	loader;
	grub_def	*grub;
	guest_disk	*disk;
	guest_nic	*nic;
	char		descr[BKPR_SZ_GUEST_DESCR];
} guest;

typedef struct guest_list {
	guest	*next;
	guest	*prev;
	guest	*data;
} guest_list;

#endif /* BKPR_TYPES_H */
