#pragma once

#include <stdint.h>
#include <stddef.h>

#define CMD_PGUP    (1 << 0)
#define CMD_PGDOWN  (1 << 1)


typedef enum VgaColor {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE,
    VGA_COLOR_GREEN,
    VGA_COLOR_CYAN,
    VGA_COLOR_RED,
    VGA_COLOR_MAGENTA,
    VGA_COLOR_BROWN,
    VGA_COLOR_LIGHT_GREY,
    VGA_COLOR_DARK_GREY,
    VGA_COLOR_LIGHT_BLUE,
    VGA_COLOR_LIGHT_CYAN,
    VGA_COLOR_LIGHT_RED,
    VGA_COLOR_LIGHT_MAGENTA,
    VGA_COLOR_WHITE,
}VgaColor;



/* The vga terminal */

typedef struct Terminal {
    size_t    row;
    size_t    column;
    uint8_t   color;
    uint16_t *buffer;
    uint16_t *cursor;
}Terminal;


void vga_tty_init();
void vga_tty_clear();
void vga_tty_load_page(uint16_t page);
void vga_tty_newline();
void vga_tty_set_cursor(uint16_t x, uint16_t y);
void vga_tty_set_color(uint8_t color);
void vga_tty_put_entry_at(char c);
void vga_tty_putchar(char c);
void vga_tty_write(const char *data, size_t size);
void vga_tty_write_string(const char *data);
void vga_tty_printf(const char *fmt, ...);
void vga_tty_repl();
