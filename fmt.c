#include "fmt.h"
#include "string.h"
#include <stdarg.h>
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


static void print_uint(uint32_t n, int base, FmtIO io) {
    static const char digits[] = "0123456789ABCDEF";

    char buf[32];
    int i = 0;

    do {
        buf[i++] = digits[n % base];
        n /= base;
    } while(n);

    switch (base) {
        case 2:
            buf[i++] = 'x';
            buf[i++] = 'b';
            break;
        case 8:
            buf[i++] = 'o';
            buf[i++] = '0';
            break;
        case 16:
            buf[i++] = 'x';
            buf[i++] = '0';
            break;
        default:
            break;
    }

    buf[i++] = '\0';

    write_string(strrev(buf), io);
}


static void print_int(int n, int base, FmtIO io) {
    if (n < 0) {
        n = -n;
    }

    print_uint(n, base, io);
}


static void print_hex(int n, FmtIO io) {
    print_int(n, 16, io);
}


static void print_uhex(uint32_t n, FmtIO io) {
    print_uint(n, 16, io);
}


/*! formatter. supports %d, %x, %p, %s, %c.
 *  The implementation should always use `tty_write_string` because it handles
 *  ansi escapes properly.
 *
 *  @fmt formatted string
 *  @... data to be formatted
 * */ 
void format(FmtIO io, const char *fmt, va_list args) {
    while(*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd') {
                print_int(va_arg(args, int), 10, io);
                fmt++;
            } else if (*fmt == 'x') {
                print_uhex(va_arg(args, int), io);
                fmt++;
            } else if (*fmt == 'p') {
                print_uhex(va_arg(args, uint32_t), io);
                fmt++;
            } else if (*fmt == 'c') {
                int c = va_arg(args, int);
                io.putchar((const char *)&c);
                fmt++;
            } else if (*fmt == 's') {
                write_string(va_arg(args, const char *), io);
                fmt++;
            }
        } else {
            fmt = io.putchar(fmt);
        }
    }
}
