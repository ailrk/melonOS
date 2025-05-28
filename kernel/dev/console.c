#include "debug.h"
#include "dev.h"
#include "err.h"
#include "kbd.h"
#include "print.h"
#include "fdefs.fwd.h"
#include "fdefs.h"
#include "trap/traps.h"
#include "dev/console.h"
#include "driver/pic.h"
#include <stdint.h>

#define BUF_SZ 1024
extern Dev devices[NDEV];

/* Ring buffer scancode for further consumption */
typedef struct Buffer {
    char data[BUF_SZ];
    uint8_t head;
    uint8_t tail;
} Buffer;


Buffer buffer = {
    .head = 0,
    .tail = 0
};


bool buffer_empty() { return buffer.head == buffer.tail; }
bool buffer_full() { return buffer.head == buffer.tail - 1; }


bool buffer_put(char value) {
    uint8_t tail = (buffer.tail + 1) % BUF_SZ;
    if (tail == buffer.head) return false;
    buffer.tail = tail;
    buffer.data[buffer.tail] = value;
    return true;
}


bool buffer_get(char *value) {
    if (buffer_empty()) {
        return false;
    }
    *value = buffer.data[buffer.head % BUF_SZ];
    buffer.head = (buffer.head + 1)  % BUF_SZ;
    return true;
}


/* Read n bytes from console to addr. Return number of bytes read. */
int console_read(Inode *ino, char *addr, int n) {
    if (ino->d.type != F_DEV || ino->d.major != DEV_CONSOLE) {
        panic("wrong device\n");
    }

    int i = 0;
    while (i < n && !buffer_empty()) {
        buffer_get(&addr[i++]);
    }

    return i;
}


int console_write(Inode *ino, char *addr, int n) {
    if (ino->d.type != F_DEV || ino->d.major != DEV_CONSOLE) {
        panic("wrong device\n");
    }

    while (n) {
        putc(addr++);
        n--;
    }
    return n;
}

void console_handler() {
    char c;
    int sc;

    /* Darin kbd ring buffer */
    while ((sc = kbd_getc()) != -1) {
        c = kbd_translate((Scancode)sc);
        if (c == -1) continue; // skip invalid char
        if (!buffer_full()) {
            buffer_put(c);
        }
    }
}


void console_init() {
    devices[DEV_CONSOLE].read = console_read;
    devices[DEV_CONSOLE].write = console_write;
    pic_irq_unmask(I_IRQ_KBD);
    pic_irq_unmask(I_IRQ_MOUSE);
}
