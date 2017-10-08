#include "bkpr.h"

extern bkpr_context	*ctx;

int
errset(int n, char *fmt, ...)
{
	va_list	va;

	ctx->err->no = n;

	if (fmt != NULL) {
		va_start(va,fmt);
		vsnprintf(ctx->err->str,sizeof(ctx->err->str),fmt,va);
		chomp(ctx->err->str);
	}

	return (ctx->err->no);
}

int
errget(void)
{

	return (ctx->err->no);
}

char *
errstr(void)
{

	return (ctx->err->str);
}

void
errclear(void)
{

	ctx->err->no = 0;
	memset(ctx->err->str,'\0',sizeof(ctx->err->str));
}

void
errprint(void)
{

	if (ctx->err->no)
		fprintf(stderr,"[*bkpr*] %s\n",errstr());
}

void
complain(char *fmt, ...)
{
	va_list	va;
	char	*p;

	if ((fmt == NULL) || (ctx->verbosity == BKPR_VERB_QUIET))
		return;

	va_start(va,fmt);
	vasprintf(&p,fmt,va);
	chomp(p);
	fprintf(stderr,"[*bkpr*] %s\n",p);
	free(p);
}

