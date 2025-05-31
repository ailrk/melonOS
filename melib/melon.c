#include "melon.h"
#include "fmt.h"
#include "sys.h"


void putc(char c) {
    write(2, &c, 1);
}


static char *putchar(char *c) { putc(*c++); return c; }


void printf(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


void vprintf(char *fmt, va_list args) {
    FmtIO io = {
        .putchar = putchar
    };

    format(io, fmt, args);
}
