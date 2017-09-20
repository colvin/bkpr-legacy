#include "bkpr.h"

extern bkpr_context_t	*ctx;

int
db_init(void)
{

	if ((ctx->db = calloc(1,sizeof(bkpr_db_t))) == NULL)
		return -1;

	ctx->db->path = strdup(BKPR_SQLITE_PATH_DEFAULT);	/* TODO */

	if (db_connect())
		return -1;

	return 0;
}

int
db_connect(void)
{

	if (ctx->db->path == NULL)
		return -1;

	if (sqlite3_open(ctx->db->path,&ctx->db->conn) != SQLITE_OK)
		return -1;

	return 0;
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

