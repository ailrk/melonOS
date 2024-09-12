#pragma once
#include <stdarg.h>

/* Generic put char function.
 *
 * A simple putchar should have type `void putchar(char c)`. But
 * such interface assumes we just want to print a character without
 * any context. Sometimes we need to know what charcter comes after
 * the current one to decide how we want to print, e.g when handling
 * escape code. So instead of passing a character, here we pass a 
 * char pointer that points to the character, so if it's an escape code
 * we can handle the case and adjust the pointer to the position of the 
 * next character to print.
 *
 * If you have a simple `void putchar(char c)`, you can adopt it as the 
 * following:
 *
 * ```c
 * static const char *putchar(const char *c) {
 *    putchar(*c++);
 *    return c;
 * }
 * ```
 * */
typedef const char *putchar_f(const char *c);


typedef struct FmtIO {
    putchar_f* putchar;
} FmtIO;


void format(FmtIO io, const char *fmt, va_list args);
