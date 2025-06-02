#include "debug.h"
#include "dev.h"
#include "err.h"
#include "print.h"
#include "string.h"
#include "fdefs.fwd.h"
#include "fdefs.h"
#include "process.h"
#include "spinlock.h"
#include "dev/console.h"
#include "uart.h"

/* Console implements the following line disciplines:
 *
 * Input buffer: Store characters typed by the user.
 * Echoing:      Print characters back to the screen as they're typed.
 * Editing:      Handle backspace (\b: 0x08, or 0x7F).
 * Termination: Return the full line when Enter is pressed
 * */


/* Handle uart escapes */
enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_ESC_BRACKET
};

#define ESC            0x1B
#define RAW_BUF_SZ    32
#define LINE_BUF_SZ    1024
extern Dev devices[NDEV];

ConsoleMode console_mode;

struct LineBuffer {
    char     data[LINE_BUF_SZ];
    SpinLock lk;
} line_buffer;

/* Raw mode uses a ring buffer. */
struct {
    char     data[RAW_BUF_SZ];
    SpinLock lk;
    int      nread;
    int      nwritten;
} raw_buffer = {
    .nread    = 0,
    .nwritten = 0
};


int console_read_cooked(Inode *ino, char *addr, int n) {
    int len    = 0;
    int cursor = 0;
    int state  = STATE_NORMAL;

    for (;;) {
        char c = uart_getc(COM1);
        switch (state) {
            case STATE_NORMAL:
                if (c == ESC) {
                    state = STATE_ESC;
                    continue;
                }

                if (c == '\n' || c == '\r') {
                    uart_putc(COM1, '\n');
                    line_buffer.data[len] = '\0';
                    return len;
                }

                if (c == '\b' || c == 0x7f) {
                    putc(" ");
                    printf("\x1b[D");
                    continue;
                }

                if (c >= 0x20 & c < 0x7f) {
                    // insert and echo char
                    if (len < LINE_BUF_SZ - 1) {
                        line_buffer.data[cursor++] = c;
                        len++;
                        uart_putc(COM1, c);
                    }
                }

                break;
            case STATE_ESC:
                if (c == '[') {
                    state = STATE_ESC_BRACKET;
                } else {
                    state = STATE_NORMAL; // ignored
                }
                break;
            case STATE_ESC_BRACKET:
                if (c == 'D') {
                }
                break;
        }
    }
}


/* Read n bytes from console to addr. Return number of bytes read. */
int console_read_raw(Inode *ino, char *addr, int n) {
    kassert(raw_buffer.nread <= raw_buffer.nwritten);

    if (ino->d.type != F_DEV || ino->d.major != DEV_CONSOLE) {
        panic("wrong device\n");
    }

    int r = 0;

    lock(&raw_buffer.lk);

    // block until read n bytes.
    while (n--) {
        // waiting for new data.
        while (raw_buffer.nread == raw_buffer.nwritten) {
            if (this_proc()->killed) {
                unlock(&raw_buffer.lk);
                return -1;
            }
            sleep(&raw_buffer.nread, &raw_buffer.lk);
        }

        // have something to read
        if (raw_buffer.nread == raw_buffer.nwritten) break;
        addr[r++] = raw_buffer.data[raw_buffer.nread++ % RAW_BUF_SZ];
    }

    unlock(&raw_buffer.lk);

    return r;
}

void console_handler_cooked() {
}

void console_handler_raw() {
    char c;

    while ((c = uart_getc(COM1)) != -1) {
        if (c < -1) continue; // skip invalid char

        // regular case.
        if (raw_buffer.nwritten == raw_buffer.nread) {
            raw_buffer.data[raw_buffer.nwritten++ % RAW_BUF_SZ] = c;
            uart_putc(COM1, c);
        }
    }

    wakeup(&raw_buffer.nread);
}


int console_read(Inode *ino, char *addr, int n) {
    switch (console_mode) {
        case CONSOLE_COOKED_MODE:
            return console_read_cooked(ino, addr, n);
            break;
        case CONSOLE_RAW_MODE:
            return console_read_raw(ino, addr, n);
            break;
    }
}


/* Write n bytes from addr to console. We simply write to the screen. */
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
    switch (console_mode) {
        case CONSOLE_COOKED_MODE:
            console_handler_cooked();
            break;
        case CONSOLE_RAW_MODE:
            console_handler_raw();
            break;
    }
}


void console_init() {
    console_mode = CONSOLE_RAW_MODE;
    line_buffer.lk = new_lock("line_buffer");
    memset(line_buffer.data, 0, LINE_BUF_SZ);

    raw_buffer.lk = new_lock("raw_buffer");
    memset(raw_buffer.data, 0, RAW_BUF_SZ);

    devices[DEV_CONSOLE].read = console_read;
    devices[DEV_CONSOLE].write = console_write;
}
