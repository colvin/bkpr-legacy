#include "bkpr.h"

extern bkpr_context	*ctx;

int
db_init(void)
{

	if ((ctx->db = calloc(1,sizeof(bkpr_db))) == NULL) {
		errset(ENOMEM,"out of memory");
		return (-1);
	}

	if (ctx->cfg->sqlite == NULL)
		ctx->cfg->sqlite = strdup(BKPR_SQLITE_PATH_DEFAULT);

	if (db_connect())
		return (-1);

	return (0);
}

int
db_connect(void)
{

	if (ctx->cfg->sqlite == NULL) {
		errset(EINVAL,"sqlite3 database path is null");
		return (-1);
	}

	if (sqlite3_open(ctx->cfg->sqlite,&ctx->db->conn) != SQLITE_OK) {
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
	}
}

#define DBERROR() \
    errset(sqlite3_errcode(ctx->db->conn),(char *)sqlite3_errmsg(ctx->db->conn))

int
db_guest_insert(guest *g)
{
	sqlite3_stmt	*guest_stmt = NULL;

	if (ctx->db->conn == NULL)
		return errset(BKPR_BAD,"database not connected");

	const char *guest_insert = "INSERT INTO guest "
	    "(name,cpu,mem,os,loader,descr,grub_map,grub_cmd) "
	    "VALUES (?,?,?,?,?,?,?,?)";

	if (sqlite3_prepare_v2(ctx->db->conn,guest_insert,-1,&guest_stmt,NULL) != SQLITE_OK)
		return (DBERROR());

	sqlite3_bind_text(guest_stmt,1,g->name,-1,NULL);
	sqlite3_bind_int(guest_stmt,2,g->cpu);
	sqlite3_bind_int(guest_stmt,3,g->mem);
	sqlite3_bind_int(guest_stmt,4,g->os);
	sqlite3_bind_int(guest_stmt,5,g->loader);
	sqlite3_bind_text(guest_stmt,6,g->descr,-1,NULL);
	sqlite3_bind_text(guest_stmt,7,g->grub->map,-1,NULL);
	sqlite3_bind_text(guest_stmt,8,g->grub->cmd,-1,NULL);

	if (sqlite3_step(guest_stmt) != SQLITE_DONE) {
		sqlite3_finalize(guest_stmt);
		return (DBERROR());
	}

	sqlite3_finalize(guest_stmt);

	/* TODO: disk, net */

	return (BKPR_GOOD);
}
