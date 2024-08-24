#include "tty.h"

#if defined(__linux__)
#error "wrong targat"
#endif

#if !defined(__i386__)
#error "wrong target. require i386"
#endif

static inline uint8_t vga_entry_color(VgaColor fg, VgaColor bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)(uc) | (uint16_t)(color) << 8;
}

size_t strlen(char const *str) {
    size_t len = 0;
    for (; *str != '\0'; ++len)
        ;
    return len;
}

static size_t const VGA_WIDTH = 80;
static size_t const VGA_HEIGHT = 25;

#define TERMBUF_START 0xb8000
#define TERMBUF_SIZE (VGA_HEIGHT * VGA_WIDTH)
#define HISTORY_START (TERMBUF_START + TERMBUF_SIZE)
#define HISTORY_END (HISTORY_START + TERMBUF_SIZE * 8)
#define MAX_PAGE 7

void memcpy(void *dest, void *src, size_t sz) {
    for (int i = 0; i < sz; ++i) {
        ((char *)dest)[i] = ((char *)src)[i];
    }
}

void clear(Terminal *self) {
    for (size_t y = 0; y < VGA_HEIGHT; ++y)
        for (size_t x = 0; x < VGA_WIDTH; ++x)
            self->buffer[y * VGA_HEIGHT + x] = vga_entry(' ', self->color);
}

void save_buffer(Terminal *self) {
    uint16_t *page_history = (uint16_t *)(HISTORY_START + TERMBUF_SIZE * self->pagen);
}

void load_page(Terminal *self, uint16_t page) {
    memcpy(self->buffer, (char *)(HISTORY_START + TERMBUF_SIZE * page), TERMBUF_SIZE);
}

void page_up(Terminal *self) {
    save_buffer(self);
    int new_pagen = self->pagen += 1;
    if (new_pagen > MAX_PAGE) {

    }
    load_page(self, self->pagen);
}

void page_down(Terminal *self) {
    save_buffer(self);
    self->pagen -= 1;
    load_page(self, self->pagen);
}

void newline(Terminal *self) {
    self->column = 0;
    self->row += 1;
}

Terminal create_terminal() {
    Terminal self = 
    { .row = 0, 
      .column = 0, 
      .color = vga_entry(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK),
      .buffer = (uint16_t *)(TERMBUF_START),
      .pagen = 0
    };

    clear(&self);
    return self;
}

void set_cursor(Terminal *self, uint16_t x, uint16_t y) {
    self->cursor = self->buffer + (y * VGA_HEIGHT + x );
}

void set_color(Terminal *self, uint8_t color) { self->color = color; }

void put_entry_at(Terminal *self, char c) {
    size_t x = self->column;
    size_t y = self->row;
    self->buffer[y * VGA_HEIGHT + x] = vga_entry(c, self->color);
}

void putchar(Terminal *self, char c) {
    if (self->row > VGA_HEIGHT) {
        page_down(self);
    }

    if (c == '\n') {
        newline(self);
        return;
    }

    put_entry_at(self, c);
    if (++self->column == VGA_WIDTH) {
        self->column = 0;
        if (++self->row == VGA_HEIGHT)
            self->row = 0;
    }
}


void write(Terminal *self, char const *data, size_t size) {
    for (size_t i = 0; i < size; ++i) putchar(self, data[i]);
}

void write_string(Terminal *self, char const *data) { write(self, data, strlen(data)); }
