#ifndef BKPR_TYPES_H
#define BKPR_TYPES_H

#define BKPR_SZ_ERRSTR		BUFSIZ

typedef enum bkpr_verbosity {
	BKPR_VERB_QUIET,
	BKPR_VERB_STD,
	BKPR_VERB_DEBUG
} bkpr_verbosity;

typedef struct bkpr_err_t {
	int	no;
	char	str[BKPR_SZ_ERRSTR];
} bkpr_err_t;

typedef struct bkpr_context_t {
	bkpr_verbosity		verbosity;
	bool			noop;
	bkpr_err_t		*err;
	DBCONN			*db;
} bkpr_context_t;

#define BKPR_SZ_GUEST_NAME	256
#define BKPR_SZ_GUEST_OS	16
#define BKPR_SZ_GUEST_LOADER	16
#define BKPR_SZ_GUEST_GRUBMAP	PATH_MAX
#define BKPR_SZ_GUEST_GRUBCMD	PATH_MAX
#define BKPR_SZ_GUEST_DESCR	256
#define BKPR_SZ_DISK_PATH	PATH_MAX

typedef enum guest_disk_type {
	DISK_TYPE_INVAL,
	DISK_TYPE_FILE,
	DISK_TYPE_ZVOL
} guest_disk_type;

#define DISK_TYPE_STR_INVAL	"invalid"
#define DISK_TYPE_STR_FILE	"file"
#define DISK_TYPE_STR_ZVOL	"zvol"

typedef struct guest_disk_t {
	int			diskid;
	int			vmid;
	guest_disk_type		type;
	char			path[BKPR_SZ_DISK_PATH];
	int			root;
	int			cloned;
	char			cloneof[BKPR_SZ_DISK_PATH];
	struct guest_disk_t	*p;
	struct guest_disk_t	*n;
} guest_disk_t;

typedef struct guest_nic_t {
	int			nicid;
	int			vmid;
	int			tap;
	int			bridge;
	struct guest_nic_t	*p;
	struct guest_nic_t	*n;
} guest_nic_t;

typedef struct guest_t {
	int		vmid;
	char		name[BKPR_SZ_GUEST_NAME];
	int		cpu;
	int		mem;
	char		os[BKPR_SZ_GUEST_OS];
	char		loader[BKPR_SZ_GUEST_LOADER];
	char		grubmap[BKPR_SZ_GUEST_GRUBMAP];
	char		grubcmd[BKPR_SZ_GUEST_GRUBCMD];
	char		descr[BKPR_SZ_GUEST_DESCR];
	guest_disk_t	*disk;
	guest_nic_t	*nic;
} guest_t;

#endif

