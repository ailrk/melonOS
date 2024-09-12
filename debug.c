#include "debug.h"
#include "fmt.h"
#include "uart.h"
#include <stdarg.h>

static const char *uart_putc1(const char *c) {
    uart_putc(*c++);
    return c;
}

void debug_printf(const char *fmt, ...) {
    FmtIO io = {
        .putchar = &uart_putc1
    };

    va_list args;
    va_start(args, fmt);
    format(io, fmt, args);
    va_end(args);
}
