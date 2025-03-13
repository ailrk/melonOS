#pragma once
#include <stdarg.h>

void debug (char *fmt, ...);
void debug_printf (char *fmt, ...);
void debug_vprintf (char *fmt, va_list args);
void debug_memdump (char *cmd, ...);
