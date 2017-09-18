#ifndef BKPR_DB_H
#define BKPR_DB_H

#ifdef DB_SQLITE
#include "/usr/local/include/sqlite3.h"
#define DBCONN	sqlite3
#endif /* SQLITE */

#ifdef DB_MYSQL
#include <my_global.h>
#include <mysql.h>
#define DBCONN	MYSQL
#endif /* MYSQL */

DBCONN	*db_connect(void);

#endif /* BKPR_DB_H */

