#include "fmt.h"
#include "string.h"
#include "ctype.h"
#include "stdlib.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

/* IO agnostic formatting functions */


/*! Write null terminated string.
 *
 *  @data   the string to write
 *  @size   size of character to write
 *  @return the pointer to the character one after the last character been printed.
 * */
static const char *write_string(const char *data, FmtIO io) {
    const char *end = data + strlen(data);
    const char *p;
    while (data < end) {
        p =  io.putchar(data);
        data = p;
    }
    return data;
}


typedef struct PrintCtl {
    bool  upper; // If true, print upper case letters in base n number;
    bool  flush; // If true, write to the output. Otherwise only return the size to output.
    bool  sign;
    int   base;  // base n
    FmtIO io;    // callback.
} PrintCtl;


/*! Print integral number  */
static int print_uint(unsigned n, PrintCtl ctl) {
    static const char digits[] = "0123456789abcdef";

    char buf[32];
    int  i = 0;

    do {
        int c =  digits[n % ctl.base];
        buf[i++] = ctl.upper ? toupper(c) : c;
        n /= ctl.base;
    } while(n);


    buf[i++] = '\0';
    strrev(buf);

    if (ctl.flush) {
        write_string(buf, ctl.io);
    }

    return strlen(buf);
}


static int print_int(int n, PrintCtl ctl) {
    if (ctl.sign) {
        if (n < 0) {
            n = -n;
        }
        return print_uint(n, ctl);
    }

    return print_uint((unsigned)n, ctl);
}



static void pad(int n, char c, FmtIO io) {
    for (int i = 0; i < n; ++i) {
        io.putchar(&c);
    }
}


/* Shorthand for print ctl */
#define PCTL_d(flush_) (PrintCtl){ .base = 10, .upper = false, .io = io, .sign = true, .flush = flush_}
#define PCTL_x(flush_) (PrintCtl){ .base = 16, .upper = false, .io = io, .sign = false, .flush = flush_}
#define PCTL_X(flush_) (PrintCtl){ .base = 16, .upper = true, .io = io, .sign = false, .flush = flush_}


/*! formatter. supports %d, %x, %X, %p, %s, %c.
 *  Syntax: %<1 char flags field><width field><format specifier>
 *  flags field:
 *      -: left aligned
 *      #: alternate form
 *      0: when width is specified, prepend 0s for numeric types.
 *  width field: integer, default right aligned. If starts with 0 pad
 *               number with 0s.
 *  format specifier:
 *      d: signed decimal.
 *      x: unsigned hexidecimal.
 *      X: unsigned hexidecimal with capital letters.
 *      p: pointer.
 *      s: null terminated string.
 *      c: character.
 *
 *  @fmt formatted string
 *  @... data to be formatted
 * */
void format(FmtIO io, const char *fmt, va_list args) {
    bool     pad0    = false;
    bool     leftpad = false;
    bool     hex     = false;
    unsigned width   = 0;

    while(*fmt) {
        if (*fmt == '%') {
            fmt++;

            // parse flags.
            {
                bool go = true;
                while (go) {
                    if (*fmt == '#') {
                        hex = true;
                        fmt++;
                    } else if (*fmt == '0') {
                        pad0 = true;
                        fmt++;
                    } else if (*fmt == '-') {
                        leftpad = true;
                        fmt++;
                    } else
                        go = false;
                }
            }

            // parse width
            width = strtol(fmt, &fmt);

#define TRY_PAD(_padn, WRITE)                      \
    do {                                           \
        if (_padn <= 0) {                          \
            WRITE;                                 \
        } else if (pad0) {                         \
            pad(_padn, '0', io);                   \
            WRITE;                                 \
        } else if (leftpad) {                      \
            pad(_padn, ' ', io);                   \
            WRITE;                                 \
        } else {                                   \
            WRITE;                                 \
            pad(_padn, ' ', io);                   \
        }                                          \
    } while (0)

            // parse format specifier
            if (*fmt == 'd') {
                int n = va_arg(args, int);
                int w = print_int(n, PCTL_d(false));
                TRY_PAD(width - w, print_int(n, PCTL_d(true)));
                fmt++;
            } else if (*fmt == 'x') {
                if (hex) write_string("0x", io);
                long n = va_arg(args, long);
                int w = print_int(n, PCTL_x(false));
                TRY_PAD(width - w, print_int(n, PCTL_x(true)));
                fmt++;
            } else if (*fmt == 'X') {
                if (hex) write_string("0x", io);
                long n = va_arg(args, long);
                int w = print_int(n, PCTL_X(false));
                TRY_PAD(width - w, print_int(n, PCTL_X(true)));
                fmt++;
            } else if (*fmt == 'p') {
                write_string("0x", io);
                long n = va_arg(args, long);
                int w = print_int(n, PCTL_X(false));
                TRY_PAD(width - w, print_int(n, PCTL_X(true)));
                fmt++;
            } else if (*fmt == 'c') {
                int c = va_arg(args, int);
                TRY_PAD(width - 1, io.putchar((const char *)&c));
                fmt++;
            } else if (*fmt == 's') {
                const char *s = va_arg(args, const char *);
                int w = strlen(s);
                TRY_PAD(width - w, write_string(s, io));
                fmt++;
            }

#undef TRY_PAD
        } else {
            fmt = io.putchar(fmt);
        }
    }
}
