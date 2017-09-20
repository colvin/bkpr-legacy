#ifndef BKPR_DB_H
#define BKPR_DB_H

#ifdef DB_SQLITE
#include "/usr/local/include/sqlite3.h"
#define DBCONN sqlite3
#define BKPR_SQLITE_PATH_DEFAULT	"/tmp/bkpr.db"
#endif /* DB_SQLITE */

#ifdef DB_MYSQL
#include <my_global.h>
#include <mysql.h>
#define DBCONN	MYSQL
#endif /* DB_MYSQL */

typedef struct bkpr_db_t {
	DBCONN		*conn;
#ifdef DB_SQLITE
	char		*path;
#endif /* DB_SQLITE */
} bkpr_db_t;

bkpr_db_type	db_type(char *);
char		*db_type_str(bkpr_db_type);

int		db_init(void);
int		db_connect(void);
void		db_disconnect(void);

#endif /* BKPR_DB_H */

