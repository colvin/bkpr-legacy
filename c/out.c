#include "bkpr.h"

extern bkpr_context	*ctx;

void
out(char *fmt, ...)
{
	va_list	va;
	char	*str;

	if (ctx->verbosity > BKPR_VERB_QUIET) {
		va_start(va,fmt);
		vasprintf(&str,fmt,va);
		fprintf(stdout,"%s %s\n","[ bkpr ]:",str);
		free(str);
		va_end(va);
	}
}

void
dbg(char *fmt, ...)
{
	va_list	va;
	char	*str;

	if (ctx->verbosity > BKPR_VERB_STD) {
		va_start(va,fmt);
		vasprintf(&str,fmt,va);
		fprintf(stderr,"%s %s\n","((bkpr)):",str);
		free(str);
		va_end(va);
	}
}
