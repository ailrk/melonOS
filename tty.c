#include "tty.h"
#include "mem.h"
#include "string.h"
#include <stdarg.h>

/**
 * VGA buffer starts at 0xb8000 and provides 256k display memory.
 */

static inline uint8_t 
vga_entry_color(VgaColor fg, VgaColor bg) {
    return fg | bg << 4;
}

static inline uint16_t 
vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)(uc) | (uint16_t)(color) << 8;
}


static size_t const VGA_WIDTH = 80;
static size_t const VGA_HEIGHT = 25;

/* after set up paging the IO space is mapped from KERN_BASE. */
#define TERMBUF_START (KERN_BASE + 0xb8000)
#define TERMBUF_SIZE (VGA_HEIGHT * VGA_WIDTH)

Terminal term;

void tty_clear() {
    term.row = 0;
    term.column = 0;
    for (size_t y = 0; y < VGA_HEIGHT; ++y)
        for (size_t x = 0; x < VGA_WIDTH; ++x)
            term.buffer[y * VGA_WIDTH + x] = vga_entry(' ', term.color);
}

void newline() {
    term.column = 0;
    term.row += 1;
}

void tty_init() {
    term.row = 0;
    term.column = 0;
    term.color = vga_entry(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    term.buffer = (uint16_t *)(TERMBUF_START);
    tty_clear();
    tty_write_string("melonos 0.0.1\n");
}

void tty_set_cursor(uint16_t x, uint16_t y) {
    term.cursor = term.buffer + (y * VGA_WIDTH + x );
}

void tty_set_color(uint8_t color) { term.color = color; }

void put_entry_at(char c) {
    size_t x = term.column;
    size_t y = term.row;
    term.buffer[y * VGA_WIDTH + x] = vga_entry(c, term.color);
}

void tty_putchar(char c) {
    if (term.row > VGA_HEIGHT) {
        tty_clear();
    }

    if (c == '\n') {
        newline();
        return;
    }

    put_entry_at(c);
    if (++term.column == VGA_WIDTH) {
        term.column = 0;
        if (++term.row == VGA_HEIGHT)
            term.row = 0;
    }
}

void tty_write(char const *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        tty_putchar(data[i]);
    }
}

void tty_write_string(char const *data) { tty_write(data, strlen(data)); }


static void print_uint(uint32_t n, int base) {
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

    tty_write_string(strrev(buf));
}


static void print_int(int n, int base) {
    if (n < 0) {
        n = -n;
        tty_putchar('-');
    }
    print_uint(n, base);
}


static void print_hex(int n) {
    print_int(n, 16);
}


static void print_uhex(uint32_t n) {
    print_uint(n, 16);
}


/* printf on vga tty.
 * supports %d, %x, %p, %s.
 * */ 
void tty_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    while(*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd') {
                print_int(va_arg(args, int), 10);
            } else if (*fmt == 'x') {
                print_uhex(va_arg(args, int));
            } else if (*fmt == 'p') {
                print_uhex(va_arg(args, uint32_t));
            } else if (*fmt == 's') {
                tty_write_string(va_arg(args, const char *));
            }
        } else {
            tty_putchar(*fmt);
        }
        fmt++;
    }
    va_end(args);
}

void repl() {
    tty_write_string("msh 0.0.1\n");
}
