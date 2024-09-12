#pragma once


void unart_init();
char uart_getc();
void uart_putc(char c);
const char *uart_write_string(const char *data);
