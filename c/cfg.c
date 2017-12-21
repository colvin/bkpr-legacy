#include "bkpr.h"

#define GET_VALUE()										\
	if ((v = strtok(NULL," \t")) == NULL)							\
		return (errset(EINVAL,"configuration parameter '%s' requires argument",k))	\

#define REDEFINE_WARN(x,y)									\
	complain("warning: cfg: redefining '%s' from %s --> %s",x,y,v);

extern bkpr_context	*ctx;

int
cfg_load(void)
{
	FILE	*fp;
	char	*lbuf, *k, *v;
	size_t	llen;

	lbuf = k = v = NULL;

	if ((ctx == NULL) || (ctx->cfgfile == NULL))
		return (errset(EINVAL,"no path to configuration file"));

	if ((ctx->cfg = cfg_alloc()) == NULL)
		err(ENOMEM,"failed to load bkpr_cfg structure");

	if ((fp = fopen(ctx->cfgfile,"r")) == NULL)
		return (errset(errno,"cannot open cfgfile %s: %s",ctx->cfgfile,strerror(errno)));

	while (getline(&lbuf,&llen,fp) != -1) {
		chomp(lbuf);
		if (strlen(lbuf) == 0)
			continue;
		if (strstr(lbuf,"#") == lbuf)
			continue;

		if ((k = strtok(lbuf," \t")) == NULL)
			continue;

		/* remember to free added cfg parameters in cfg_free() */

		if (CMP(k,"prefix")) {
			GET_VALUE();
			if (ctx->cfg->prefix != NULL) {
				REDEFINE_WARN("prefix",ctx->cfg->prefix);
				free(ctx->cfg->prefix);
			}
			ctx->cfg->prefix = strdup(v);
			dbg("cfg: 'prefix' --> %s",ctx->cfg->prefix);
		} else if (CMP(k,"zfs-prefix")) {
			GET_VALUE();
			if (ctx->cfg->zprefix != NULL) {
				REDEFINE_WARN("zfs-prefix",ctx->cfg->zprefix);
				free(ctx->cfg->zprefix);
			}
			ctx->cfg->zprefix = strdup(v);
			dbg("cfg: 'zfs-prefix' --> %s",ctx->cfg->zprefix);
#ifdef DB_SQLITE
		} else if (CMP(k,"sqlite")) {
			GET_VALUE();
			if (ctx->cfg->sqlite != NULL) {
				REDEFINE_WARN("sqlite",ctx->cfg->sqlite);
				free(ctx->cfg->sqlite);
			}
			ctx->cfg->sqlite = strdup(v);
			dbg("cfg: 'sqlite' --> %s",ctx->cfg->sqlite);
#endif /* DB_SQLITE */
		} else {
			complain("cfg: skipping unknown parameter '%s'",k);
		}
	}

	free(lbuf);

	fclose(fp);

	return (BKPR_GOOD);
}
