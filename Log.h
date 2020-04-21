#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>
#include <stdbool.h>


void Log_set_flush_after_printing(bool value);
void Log_set_do_log(bool value);
void Log(const char* category, const char* format, ...);
void Log_error(const char* category, const char* format, ...);


#endif
