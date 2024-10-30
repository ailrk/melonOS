#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "mem.h"
#include "ansi.h"
#include "fmt.h"
#include "driver/vga.h"


/* The vga screen */
typedef struct Screen {
    size_t    row;
    size_t    column;
    uint8_t   color;
    uint16_t *buffer;
    uint16_t *cursor;
}Screen;


Screen screen;


static inline uint8_t
vga_entry_color (VgaColor fg, VgaColor bg) {
    return fg | bg << 4;
}


static inline uint16_t
vga_entry (unsigned char uc, uint8_t color) {
    return (uint16_t)(uc) | (uint16_t)(color) << 8;
}


static size_t const VGA_WIDTH  = 80;
static size_t const VGA_HEIGHT = 25;


/* after set up paging the IO space is mapped from KERN_BASE. */
#define TERMBUF_START (KERN_BASE + 0xb8000)
#define TERMBUF_SIZE (VGA_HEIGHT * VGA_WIDTH)


static void set_bg_color (VgaColor bg) {
    screen.color |= bg << 4;
}


static void set_fg_color (VgaColor fg) {
    screen.color = vga_entry_color (fg, screen.color >> 4);
}


void vga_clear () {
    screen.row    = 0;
    screen.column = 0;
    for (size_t y = 0; y < VGA_HEIGHT; ++y)
        for (size_t x = 0; x < VGA_WIDTH; ++x)
            screen.buffer[y * VGA_WIDTH + x] = vga_entry(' ', screen.color);
}


void newline () {
    screen.column = 0;
    screen.row += 1;
}


void vga_init () {
    screen.row    = 0;
    screen.column = 0;
    screen.color  = vga_entry_color (VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    screen.buffer = (uint16_t *)(TERMBUF_START);
    vga_clear ();
    vga_printf ("melonos 0.0.1\n");
}


void vga_set_cursor (uint16_t x, uint16_t y) {
    screen.cursor = screen.buffer + (y * VGA_WIDTH + x );
}


void vga_set_color (uint8_t color) { screen.color = color; }


void put_entry_at(char c) {
    size_t x = screen.column;
    size_t y = screen.row;
    screen.buffer[y * VGA_WIDTH + x] = vga_entry(c, screen.color);
}


void vga_putchar(char c) {
    if (screen.row > VGA_HEIGHT) {
        vga_clear ();
    }

    if (c == '\n') {
        newline ();
        return;
    }

    put_entry_at (c);
    if (++screen.column == VGA_WIDTH) {
        screen.column = 0;
        if (++screen.row == VGA_HEIGHT)
            screen.row = 0;
    }
}


static void ansi_cntl(const ANSIState *ansi) {
    switch(ansi->tag) {
        case ANSI_COLOR:
            switch (ansi->value.color) {
            case ANSIColor_BBLK:
                set_bg_color (VGA_COLOR_BLACK);
                break;
            case ANSIColor_BRED:
                set_bg_color (VGA_COLOR_RED);
                break;
            case ANSIColor_BGRN:
                set_bg_color (VGA_COLOR_GREEN);
                break;
            case ANSIColor_BYLW:
                set_bg_color (VGA_COLOR_LIGHT_BROWN);
                break;
            case ANSIColor_BBLU:
                set_bg_color (VGA_COLOR_BLUE);
                break;
            case ANSIColor_BMAG:
                set_bg_color (VGA_COLOR_LIGHT_MAGENTA);
                break;
            case ANSIColor_BCYA:
                set_bg_color (VGA_COLOR_CYAN);
                break;
            case ANSIColor_BWHT:
                set_bg_color (VGA_COLOR_WHITE);
                break;
            case ANSIColor_BDEF:
                set_bg_color (VGA_COLOR_BLACK);
                break;
            case ANSIColor_FBLK:
                set_fg_color (VGA_COLOR_BLACK);
                break;
            case ANSIColor_FRED:
                set_fg_color (VGA_COLOR_RED);
                break;
            case ANSIColor_FGRN:
                set_fg_color (VGA_COLOR_GREEN);
                break;
            case ANSIColor_FYLW:
                set_fg_color (VGA_COLOR_LIGHT_BROWN);
                break;
            case ANSIColor_FBLU:
                set_fg_color (VGA_COLOR_BLUE);
                break;
            case ANSIColor_FMAG:
                set_fg_color (VGA_COLOR_LIGHT_MAGENTA);
                break;
            case ANSIColor_FCYA:
                set_fg_color (VGA_COLOR_CYAN);
                break;
            case ANSIColor_FWHT:
                set_fg_color (VGA_COLOR_WHITE);
                break;
            case ANSIColor_FDEF:
                set_fg_color (VGA_COLOR_WHITE);
                break;
            case ANSIColor_RES:
                screen.color = vga_entry_color (VGA_COLOR_WHITE, VGA_COLOR_BLACK);
                break;
            default: break;
            }
            break;
        case ANSI_CURSOR : break;
        case ANSI_ERASE : break;
    }
}


/*! Write a single char to the screen. If it encounters an escape code, consume
 *  it first then print the next character.
 * */
char *vga_writec (char *data) {
    ANSIState ansi;

    if (*data == '\033') { // ansi
        char *p;
        if ((p = ansi_parse (&ansi, data)) != 0) {
            ansi_cntl (&ansi);
            data = p;
        }
    }

    vga_putchar (*data++);
    return data;
}


/*! The implementation should always use `vga_write_string` because it handles
 *  ansi escapes properly.
 *
 *  `vga_printf` is driven by `format` and it supports all it's format specifiers.
 *
 *  @fmt formatted string
 *  @... data to be formatted
 * */
void vga_printf(char *fmt, ...) {
    FmtIO io = {
        .putchar = &vga_writec,
    };
    va_list args;
    va_start (args, fmt);
    format (io, fmt, args);
    va_end (args);
}
