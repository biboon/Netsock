#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif


static FILE *output;


int log_start(const char *filepath)
{
	output = (filepath != NULL) ? fopen(filepath, "a") : stdout;
	return (output != NULL) ? 0 : -1;
}

int log_end(void)
{
	return (output != NULL) ? fclose(output) : 0;
}


void log_(const char *level, const char *fmt, ...)
{
	char date[18], msg[80];
	time_t tloc = time(NULL);
	struct tm *time_info = localtime(&tloc);
	if (unlikely(strftime(date, sizeof date, "%y-%m-%d %H:%M:%S", time_info) == 0))
		return;
	va_list arg;
	va_start(arg, fmt);
	if (unlikely(vsnprintf(msg, sizeof msg, fmt, arg) < 0))
		return;
	va_end(arg);
	fprintf(output, "(%17s%s) %s\n", date, level, msg);
}
