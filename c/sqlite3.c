#include "bkpr.h"

extern bkpr_context	*ctx;

int
db_init(void)
{

	if ((ctx->db = calloc(1,sizeof(bkpr_db))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (-1);
	}

	ctx->db->path = strdup(BKPR_SQLITE_PATH_DEFAULT);	/* TODO */

	if (db_connect())
		return (-1);

	return (0);
}

int
db_connect(void)
{

	if (ctx->db->path == NULL) {
		errset(EINVAL,"sqlite3 database path is null");
		return (-1);
	}

	if (sqlite3_open(ctx->db->path,&ctx->db->conn) != SQLITE_OK) {
		errset(EINVAL,"db connect failed: %s\n",sqlite3_errmsg(ctx->db->conn));
		db_disconnect();
		return (-1);
	}

	return (0);
}

void
db_disconnect(void)
{

	if ((ctx->db != NULL) && (ctx->db->conn != NULL)) {
		sqlite3_close(ctx->db->conn);
		ctx->db->conn = NULL;
		free(ctx->db->path);
	}
}

