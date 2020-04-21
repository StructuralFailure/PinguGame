#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "Log.h"

#define TC_RED     "\x1B[31m"
#define TC_DEFAULT "\x1B[0m"


bool flush_after_printing = false;
bool do_log = true;


void Log_set_flush_after_printing(bool value) 
{
	flush_after_printing = value;
}


void Log_set_do_log(bool value) 
{
	do_log = value;
}


void Log(const char* category, const char* format, ...)
{
	if (!do_log) {
		return;
	}

	va_list args;

	printf("[%s] ", category);

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	
	printf("\n");

	if (flush_after_printing) {
		fflush(stdout);
	}
}

void Log_error(const char* category, const char* format, ...)
{
	if (!do_log) {
		return;
	}

	va_list args;

	printf("%s[%s]", TC_RED, category);

	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("%s\n", TC_DEFAULT);

	if (flush_after_printing) {
		fflush(stdout);
	}
}