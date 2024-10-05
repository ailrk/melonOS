#pragma once
#include <stdint.h>

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



void  vga_init();
void  vga_clear();
void  vga_load_page(uint16_t page);
void  vga_newline();
void  vga_set_cursor(uint16_t x, uint16_t y);
void  vga_set_color(uint8_t color);
void  vga_put_entry_at(char c);
void  vga_putchar(char c);
char *vga_writec(char *data);
void  vga_printf(char *fmt, ...);
