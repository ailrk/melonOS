#pragma once
#include <stdarg.h>

/* The melonos user library */

void  putc(char c);
char *gets(char *buf, int nmax);
void  printf(char *fmt, ...);
void  vprintf(char *fmt, va_list args);
