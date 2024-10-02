#pragma once
#include <stdint.h>
#include <stddef.h>

#define CMD_PGUP    (1 << 0)
#define CMD_PGDOWN  (1 << 1)


typedef enum VgaColor {
	VGA_COLOR_BLACK         = 0,
	VGA_COLOR_BLUE          = 1,
	VGA_COLOR_GREEN         = 2,
	VGA_COLOR_CYAN          = 3,
	VGA_COLOR_RED           = 4,
	VGA_COLOR_MAGENTA       = 5,
	VGA_COLOR_BROWN         = 6,
	VGA_COLOR_LIGHT_GREY    = 7,
	VGA_COLOR_DARK_GREY     = 8,
	VGA_COLOR_LIGHT_BLUE    = 9,
	VGA_COLOR_LIGHT_GREEN   = 10,
	VGA_COLOR_LIGHT_CYAN    = 11,
	VGA_COLOR_LIGHT_RED     = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN   = 14,
	VGA_COLOR_WHITE         = 15,
}VgaColor;



/* The vga terminal */
typedef struct Terminal {
    size_t    row;
    size_t    column;
    uint8_t   color;
    uint16_t *buffer;
    uint16_t *cursor;
}Terminal;


void  tty_init();
void  tty_clear();
void  tty_load_page(uint16_t page);
void  tty_newline();
void  tty_set_cursor(uint16_t x, uint16_t y);
void  tty_set_color(uint8_t color);
void  tty_put_entry_at(char c);
void  tty_putchar(char c);
char *tty_writec(char *data);
void  tty_printf(char *fmt, ...);
void  tty_repl();
