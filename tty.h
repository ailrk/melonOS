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
    size_t row;
    size_t column;
    uint8_t color;
    uint16_t *buffer;
    uint16_t pagen;
    uint16_t *cursor;
}Terminal;


struct Terminal create_teriminal();
void clear(Terminal *self);
void save_buffer(Terminal *self);
void load_page(Terminal *self, uint16_t page);
void page_up(Terminal *self);
void page_down(Terminal *self);
void newline(Terminal *self);
void set_cursor(Terminal *self, uint16_t x, uint16_t y);
void set_color(Terminal*, uint8_t color);
void put_entry_at(Terminal*, char c);
void putchar(Terminal*, char c);
void write(Terminal*, const char *data, size_t size);
void write_string(Terminal*, const char *data);
void repl(Terminal*);
