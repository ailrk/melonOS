#include <stdarg.h>
#include "print.h"
#include "uart.h"
#include "vga.h"


void printf(char *fmt, ...) {
    va_list args;
    va_start (args, fmt);
    vprintf(fmt, args);
    va_end (args);
}

void putc(char *addr) {
    vga_writec(addr);
    uart_putc(COM1, *addr);
}


void vprintf(char *fmt, va_list args) {
    uart_vprintf(COM1, fmt, args);
    vga_vprintf(fmt, args);
}
