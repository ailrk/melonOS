#include <stdarg.h>
#include <stdint.h>
#include "debug.h"
#include "fmt.h"
#include "stdlib.h"
#include "ctype.h"
#include "errno.h"
#include "driver/uart.h"


static char *uart_putc1 (char *c) {
    uart_putc(*c++);
    return c;
}


void debug_printf (char *fmt, ...) {
    FmtIO io = {
        .putchar = &uart_putc1
    };

    va_list args;
    va_start (args, fmt);
    format (io, fmt, args);
    va_end (args);
}


/*! gdb style memory dump
 *  Syntax: nfu addr, where
 *      n repeat count
 *      f display format
 *      u size
 *  possible formats:
 *      x, d
 *  possible size value:
 *      b  byte
 *      h  halfword
 *      w  word
 *      g  gianword
 *  e.g:
 *      debug_memdump("10b 0x1000");
 *      debug_memdump("100xw %p", (char *)ptr);
 *      debug_memdump("10b %p", (char *)ptr);
 *      debug_memdump("10dg %p", (char *)ptr);
 * */
void debug_memdump (char *cmd, ...) {
    char *addr;
    unsigned n = 1;
    char f     = 'x';
    char sz    = 1;

    va_list args;
    va_start (args, cmd);

    char *endptr;

    n = strtol (cmd, &endptr);
    if (n == 0 && errno == ERANGE) {
        debug_printf ("invalid repeat count\n");
    }

    while (*endptr && !isspace (*endptr)) {
        switch (*endptr) {
            case 'x':
                f = 'x';
                break;
            case 'd':
                f = 'd';
                break;
            case 'b':
                sz = 1;
                break;
            case 'h':
                sz = 2;
                break;
            case 'w':
                sz = 4;
                break;
            case 'g':
                sz = 8;
                break;
            default:
                break;
        }
        endptr++;
    }

    while (isspace (*endptr)) endptr++;

    if (*endptr == '%') {
        endptr++;
        if (*endptr == 'p') {
            addr = va_arg (args, char *);
        }
    } else {
        addr = (char *)strtol (endptr, 0);
    }
    va_end(args);

#define sp(n) { if (i == 0 || i % n) debug_printf(" "); else debug_printf("\n"); }
    for (unsigned i = 0; i < n; ++i) {
        char *p = &addr[i * sz];
        switch (sz) {
            case 1:
                debug_printf ("%#02x", *(uint8_t*)(p));
                sp(20);
                continue;
            case 2:
                debug_printf("%#04x", * (uint16_t*)p);
                sp(15);
                continue;
            case 4:
                debug_printf ("%#08x", *(uint32_t*)p);
                sp(10);
                continue;
            case 8:
                debug_printf ("%#016x", *(uint64_t*)p);
                sp(4);
                continue;
            default:
                debug_printf ("invalid sz\n");
                return;
        }
    }
#undef sp
    debug_printf ("\n");
}
