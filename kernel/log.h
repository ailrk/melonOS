#pragma once
#include <stdarg.h>

void log (char *fmt, ...);
void log_printf (char *fmt, ...);
void log_vprintf (char *fmt, va_list args);
