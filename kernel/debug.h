#pragma once
#include <stdarg.h>

void debug(char *fmt, ...);
char *debug_putc(char *c);
void debug_printf(char *fmt, ...);
void debug_vprintf(char *fmt, va_list args);
void debug_memdump(char *cmd, ...);
