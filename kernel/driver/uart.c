#include <stdbool.h>
#include <stdint.h>
#include "err.h"
#include "uart.h"
#include "fmt.h"
#include "i386.h"


#define BUF_SZ 256


/* Circular buffer scancode for further consumption */
typedef struct UArtBuffer {
    char    data[BUF_SZ];
    uint8_t head;
    uint8_t tail;
} UArtBuffer;


UArtBuffer com1_buffer = { .head = 0, .tail = 0 };
UArtBuffer com2_buffer = { .head = 0, .tail = 0 };


UArtBuffer *get_buffer(uint16_t com) {
    switch (com) {
        case COM1:
            return &com1_buffer;
        case COM2:
            return &com2_buffer;
        default:
            return 0;
    }
}


bool buffer_empty(uint16_t com) {
    UArtBuffer *buffer = get_buffer(com);
    return buffer->head == buffer->tail;
}


bool buffer_put(uint16_t com, char value) {
    UArtBuffer *buffer = get_buffer(com);
    uint8_t tail = (buffer->tail + 1) % BUF_SZ;
    if (tail == buffer->head) return false;
    buffer->tail = tail;
    buffer->data[buffer->tail] = value;
    return true;
}


bool buffer_get(uint16_t com, char *value) {
    UArtBuffer *buffer = get_buffer(com);
    if (buffer_empty(com)) {
        return false;
    }
    *value = buffer->data[buffer->head % BUF_SZ];
    buffer->head = (buffer->head + 1)  % BUF_SZ;
    return true;
}


static bool init_serial(uint16_t com) {
   outb(com + 1, 0x00);    // Disable all interrupts
   outb(com + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(com + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(com + 1, 0x00);    //                  (hi byte)
   outb(com + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(com + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(com + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(com + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(com + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty (i.e: not same byte as sent)
   if(inb(com + 0) != 0xAE) {
      return false;
   }

   outb(com + 1, 0x01);  // Enable Received Data Available interrupt

   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   outb(com + 4, 0x0F);
   return true;
}


void uart_init(uint16_t com) {
    if (!init_serial(com)) {
        panic("init serial");
    }
}


static int is_transmit_empty(uint16_t com) {
   return inb(com + 5) & 0x20;
}


static int serial_received(uint16_t com) {
   return inb(com + 5) & 1;
}


static char read_serial(uint16_t com) {
   while (serial_received(com) == 0);
   return inb(com);
}


static void write_serial(uint16_t com, char a) {
   while (is_transmit_empty(com) == 0);
   outb(com,a);
}


void uart_read(uint16_t com) {
    char c = read_serial(com);
    buffer_put(com, c);
}


char uart_getc(uint16_t com) {
    char c;
    if (buffer_get(com, &c)) {
        return c;
    }
    return -1;
}


void uart_putc(uint16_t com, char c) {
    write_serial(com, c);
}


static char *uart_putc_com1(char *c) { uart_putc(COM1, *c++); return c; }
static char *uart_putc_com2(char *c) { uart_putc(COM2, *c++); return c; }


void uart_vprintf(uint16_t com,char *fmt, va_list args) {
    char *(*putc)(char *) = 0;
    switch (com) {
        case COM1:
            putc = uart_putc_com1;
            break;
        case COM2:
            putc = uart_putc_com2;
            break;
        default:
            panic("invalid uart port\n");
            break;
    }

    FmtIO io = {
        .putchar = putc
    };

    format(io, fmt, args);
}
