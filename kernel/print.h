#pragma once
#include <stdarg.h>


#define LOG_BOOT "[\033[32mboot\033[0m]"
#define LOG_OK "\033[32mok\033[0m"

void printf(char *fmt, ...);
void vprintf(char *fmt, va_list args);
void dprintf(char *fmt, ...);
void dvprintf(char *fmt, va_list args);
void putc(char *addr);
