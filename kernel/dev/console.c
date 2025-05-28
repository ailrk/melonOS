#include "debug.h"
#include "dev.h"
#include "print.h"
#include "fdefs.fwd.h"
#include "fdefs.h"
#include "trap/traps.h"
#include "dev/console.h"
#include "driver/pic.h"

extern Dev devices[NDEV];


int console_read(Inode *ino, char *addr, int n) {
    return -1;
}


int console_write(Inode *ino, char *addr, int n) {
    while (n) {
        putc(addr);
        addr++;
        n--;
    }
}


void console_handler(console_getc_t getc) {
    char c;
    if ((c = getc()) != -1) {
        printf("%c", c);
        debug_printf("%c", c);
    }
}


void console_init() {
    devices[DEV_CONSOLE].read = console_read;
    devices[DEV_CONSOLE].write = console_write;
    pic_irq_unmask(I_IRQ_KBD);
    pic_irq_unmask(I_IRQ_MOUSE);
}
