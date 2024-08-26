#include "tty.h"
#include "string.h"
#include "i386.h"
#include "defs.h"

#if defined(__linux__)
#error "wrong targat"
#endif

#if !defined(__i386__)
#error "wrong target. require i386"
#endif

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

#define TERMBUF_START 0xb8000
#define TERMBUF_SIZE (VGA_HEIGHT * VGA_WIDTH)
#define HISTORY_START (TERMBUF_START + TERMBUF_SIZE)
#define HISTORY_END (HISTORY_START + TERMBUF_SIZE * 8)
#define MAX_PAGE 7

Terminal term;

void vga_tty_clear() {
    for (size_t y = 0; y < VGA_HEIGHT; ++y)
        for (size_t x = 0; x < VGA_WIDTH; ++x)
            term.buffer[y * VGA_WIDTH + x] = vga_entry(' ', term.color);
}

void vga_tty_save_buffer() {
    uint16_t *page_history = (uint16_t *)(HISTORY_START + TERMBUF_SIZE * term.pagen);
}

void vga_tty_load_page(uint16_t page) {
    memcpy(term.buffer, (char *)(HISTORY_START + TERMBUF_SIZE * page), TERMBUF_SIZE);
}

void vga_tty_page_up() {
    vga_tty_save_buffer();
    int new_pagen = term.pagen - 1;
    if (new_pagen < 0)
        term.pagen = 0;
    else
        term.pagen = new_pagen;
    vga_tty_load_page(term.pagen);
}

void vga_tty_page_down() {
    vga_tty_save_buffer();
    int new_pagen = term.pagen + 1;
    if (new_pagen > MAX_PAGE)
        term.pagen = MAX_PAGE;
    else 
        term.pagen = new_pagen;
    vga_tty_load_page(term.pagen);
}

void newline() {
    term.column = 0;
    term.row += 1;
}

void vga_tty_init() {
    term.row = 0;
    term.column = 0;
    term.color = vga_entry(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    term.buffer = (uint16_t *)(TERMBUF_START);
    term.pagen = 0;
    vga_tty_clear();
}

void vga_tty_set_cursor(uint16_t x, uint16_t y) {
    term.cursor = term.buffer + (y * VGA_WIDTH + x );
}

void vga_tty_set_color(uint8_t color) { term.color = color; }

void put_entry_at(char c) {
    size_t x = term.column;
    size_t y = term.row;
    term.buffer[y * VGA_WIDTH + x] = vga_entry(c, term.color);
}

void vga_tty_putchar(char c) {
    if (term.row > VGA_HEIGHT) {
        vga_tty_page_down();
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


void vga_tty_write(char const *data, size_t size) {
    for (size_t i = 0; i < size; ++i) vga_tty_putchar(data[i]);
}

void vga_tty_write_string(char const *data) { vga_tty_write(data, strlen(data)); }

void repl() {
    vga_tty_write_string("msh 0.0.1\n");
}
