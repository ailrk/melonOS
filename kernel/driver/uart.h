#pragma once
#include <stdint.h>
#include <stdarg.h>

#define COM1  0x3f8
#define COM2  0x2F8


void uart_init(uint16_t com);
char uart_getc(uint16_t com);
void uart_putc(uint16_t com, char c);
void uart_vprintf(uint16_t com,char *fmt, va_list args);
