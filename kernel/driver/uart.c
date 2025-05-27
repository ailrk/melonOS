#include <stdbool.h>
#include <stdint.h>
#include "err.h"
#include "uart.h"
#include "i386.h"


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


static int serial_received(uint16_t com) {
   return inb(com + 5) & 1;
}


static char read_serial(uint16_t com) {
   while (serial_received(com) == 0);
   return inb(com);
}


char uart_getc(uint16_t com) {
    return read_serial(com);
}


static int is_transmit_empty(uint16_t com) {
   return inb(com + 5) & 0x20;
}


static void write_serial(uint16_t com, char a) {
   while (is_transmit_empty(com) == 0);
   outb(com,a);
}


void uart_putc(uint16_t com, char c) {
    write_serial(com, c);
}
