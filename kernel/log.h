#pragma once
#include <stdarg.h>


#define LOG_BOOT "[\033[32mboot\033[0m]"
#define LOG_OK "\033[32mok\033[0m"

void log(char *fmt, ...);
void log_printf(char *fmt, ...);
void log_vprintf(char *fmt, va_list args);
