#include "debug.h"
#include "dev.h"
#include "err.h"
#include "kbd.h"
#include "print.h"
#include "string.h"
#include "fdefs.fwd.h"
#include "fdefs.h"
#include "process.h"
#include "spinlock.h"
#include "stdlib.h"
#include "trap/traps.h"
#include "dev/console.h"
#include "driver/pic.h"
#include "uart.h"

#define BUF_SZ 1024
extern Dev devices[NDEV];

/* Ring buffer for further consumption */
typedef struct Buffer {
    char     data[BUF_SZ];
    SpinLock lk;
    int      nread;
    int      nwritten;
} Buffer;


Buffer buffer;


bool buffer_full(void) { return buffer.nwritten == buffer.nread + BUF_SZ; }



/* Read n bytes from console to addr. Return number of bytes read. */
int console_read(Inode *ino, char *addr, int n) {
    kassert(buffer.nread <= buffer.nwritten);

    if (ino->d.type != F_DEV || ino->d.major != DEV_CONSOLE) {
        panic("wrong device\n");
    }

    int r = 0;

    lock(&buffer.lk);

    // block until read n bytes.
    while (n--) {
        // waiting for new data.
        while (buffer.nread == buffer.nwritten) {
            if (this_proc()->killed) {
                unlock(&buffer.lk);
                return -1;
            }
            sleep(&buffer.nread, &buffer.lk);
        }

        // have something to read
        if (buffer.nread == buffer.nwritten) break;
        addr[r++] = buffer.data[buffer.nread++ % BUF_SZ];
    }

    unlock(&buffer.lk);

    return r;
}


/* Write n bytes from addr to console. We simply write to the screen.
 * */
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

    /* Polling kbd ring buffer. */
    while ((sc = kbd_getscancode()) != -1) {
        c = kbd_translate((Scancode)sc);
        if (c == -1) continue; // skip invalid char
        if (!buffer_full()) {
            buffer.data[buffer.nwritten++ % BUF_SZ] = c;
        }
    }

    /* Poling uart ring buffer. */
    while ((c = uart_getc(COM1)) != -1) {
        if (c == -1) continue; // skip invalid char
        if (!buffer_full()) {
            buffer.data[buffer.nwritten++ % BUF_SZ] = c;
        }
    }

#ifdef DEBUG
        if (buffer_full()) {
            debug("Console buffer is full\n");
        }
#endif

    wakeup(&buffer.nread);
}


void console_init() {
    buffer.nread = 0;
    buffer.nwritten = 0;
    buffer.lk = new_lock("console_buf");
    memset(buffer.data, 0, BUF_SZ);

    devices[DEV_CONSOLE].read = console_read;
    devices[DEV_CONSOLE].write = console_write;
    pic_irq_unmask(I_IRQ_KBD);
    pic_irq_unmask(I_IRQ_MOUSE);
}
