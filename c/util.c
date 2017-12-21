#include "bkpr.h"

extern bkpr_context	*ctx;

bkpr_context *
ctx_alloc(void)
{
	bkpr_context	*p;

	if ((p = calloc(1,sizeof(bkpr_context))) == NULL)
		return (NULL);

	return (p);
}

bkpr_err *
err_alloc(void)
{
	bkpr_err	*p;

	if ((p = calloc(1,sizeof(bkpr_err))) == NULL)
		return (NULL);

	return (p);
}

bkpr_cfg *
cfg_alloc(void)
{
	bkpr_cfg	*p;

	if ((p = calloc(1,sizeof(bkpr_cfg))) == NULL)
		return (NULL);

	return (p);
}

void
cfg_free(void)
{
	if ((ctx == NULL) || (ctx->cfg == NULL))
		return;

	free(ctx->cfg->prefix);
	free(ctx->cfg->zprefix);

	free(ctx->cfg);
}


bkpr_db_type
db_type(char *str)
{

#define DBTYPECMP(x)	(strcmp(str,x) == 0)

	if (DBTYPECMP("sqlite"))
		return (BKPR_DBTYPE_SQLITE);
	else if (DBTYPECMP("mysql"))
		return (BKPR_DBTYPE_MYSQL);
	else
		return (BKPR_DBTYPE_INVAL);
}

char * /* must free */
db_type_str(bkpr_db_type type)
{
	char	*str;

	switch(type) {
		case BKPR_DBTYPE_INVAL:
			str = strdup("invalid");
			break;
		case BKPR_DBTYPE_SQLITE:
			str = strdup("sqlite");
			break;
		case BKPR_DBTYPE_MYSQL:
			str = strdup("mysql");
			break;
		default:
			str = strdup("invalid");
			break;
	}

	return (str);
}

int
chomp(char *str)
{
	int	c = 0;
	size_t	l = 0;

	if (str == NULL)
		return (0);

	l = strlen(str);
	if (l == 0)
		return (0);

	while (str[l-1] == '\n') {
		str[l-1] = '\0';
		l--;
		c++;
	}

	return (c);
}

char *	/* must free */
lc(char *in)
{
	int	c;
	char	*out;

	if ((in == NULL) || (strlen(in) == 0))
		return (NULL);

	out = strdup(in);

	for (int i = 0; i < strlen(in); i++)
		out[i] = tolower(out[i]);

	return (out);
}

int
str_isnumber(char *str)
{
	size_t	len;

	if (str == NULL)
		return 0;

	len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (isnumber(str[i]) == 0)
			return 0;
	}

	return 1;
}
