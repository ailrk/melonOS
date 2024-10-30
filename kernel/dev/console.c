#include "debug.h"
#include "dev.h"
#include "fdefs.h"
#include "trap/traps.h"
#include "dev/console.h"
#include "driver/vga.h"
#include "driver/pic.h"

extern Dev devices[NDEV];


void console_handler (console_getc_t getc) {
    char c;
    if ((c = getc ()) != -1) {
        vga_printf ("%c", c);
        debug_printf ("%c", c);
    }
}


int console_read (Inode *ino, char *addr, int n) {
    return -1;
}


int console_write (Inode *ino, char *addr, int n) {
    while (n) {
        vga_writec (addr++);
        n--;
    }
}


void console_init () {
    devices[DEV_CONSOLE].read = console_read;
    devices[DEV_CONSOLE].write = console_write;
    pic_irq_unmask(I_IRQ_KBD);
    pic_irq_unmask(I_IRQ_MOUSE);
}
