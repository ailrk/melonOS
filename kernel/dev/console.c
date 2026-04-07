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

/* ------------------------------
 * There are two console modes.
 *
 * Cooked Mode:
 *
 * The default mode with the behavior you'd expect from a standard shell.
 * The kernel buffers the input and only hands it over to the user program when
 * the user presses Enter.
 *
 * - Input buffer: Store characters typed by the user.
 * - Echoing:      Print characters back to the screen as they're typed.
 * - Editing:      Handle backspace (\b: 0x08, or 0x7F).
 * - Termination:  Return the full line when Enter is pressed
 *
 * Raw Mode:
 *
 * The console acts like a dumb pipe with zero processing.
 *
 * - Immediate:  As soon as a byte arrived, it's placed in buffer immediately.
 * - No Echo:    Nothing appear on the screen.
 * - No Editing: No character escaping.
 * */


/* Handle uart escapes */
enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_ESC_BRACKET
};

#define ESC           0x1B
#define RAW_BUF_SZ    32
#define LINE_BUF_SZ   1024
extern Dev devices[NDEV];

ConsoleMode console_mode;


/* Line mode buffer. Only commit the date when hits a '\n'. */
struct LineBuffer {
    char     data[LINE_BUF_SZ];
    SpinLock lk;
    int      len;
    int      cursor;
    int      state;
    int      nnewline; // number of newlines in the buffer.
} line_buffer = {
    .len      = 0,
    .cursor   = 0,
    .state    = STATE_NORMAL,
    .nnewline = 0
};


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


/* ------------------------------
 * Cooked mode
 */

int console_read_cooked(Inode *ino, char *addr, int n) {
    int r = 0;

    if (ino->d.type != F_DEV || ino->d.major != DEV_CONSOLE) {
        panic("wrong device\n");
    }

    lock(&line_buffer.lk);

    // Wait until there is at least one new line in the buffer.
    while (line_buffer.nnewline == 0) {
        debug("waiting to read, nnewline %d\n", line_buffer.nnewline);
        sleep(&line_buffer, &line_buffer.lk);
    }
    debug("console read woke, reading\n");

    // Copy from line_buffer.data to addr
    while (r < n && r < line_buffer.len) {
        char c = line_buffer.data[r];
        addr[r++] = c;
        if (c == '\n') break;
    }

    line_buffer.nnewline--;

    // Shift or Reset Reset
    if (line_buffer.len > r) {
        memmove(line_buffer.data, &line_buffer.data[r], line_buffer.len - r);
        line_buffer.len -= r;
        line_buffer.cursor = line_buffer.len;
    } else {
        line_buffer.len = 0;
        line_buffer.cursor = 0;
    }

    unlock(&line_buffer.lk);
    return r;
}


/* Cooked mode handler. */
void console_handler_cooked() {

    lock(&line_buffer.lk);

    for (;;) {
        char c = uart_getc(COM1);
        if (c == -1) break; // Break the loop

        switch (line_buffer.state) {
            case STATE_NORMAL:
                if (c == ESC) {
                    line_buffer.state = STATE_ESC;
                    continue;
                }

                if (c == '\r') c = '\n';

                // Cooked mode retain the newline in the buffer.
                if (c == '\n') {
                    uart_putc(COM1, '\n');
                    line_buffer.data[line_buffer.len++] = '\n';
                    line_buffer.data[line_buffer.len] = '\0';
                    line_buffer.nnewline++;

                    debug("console_handler_cooked wakup %d \n", line_buffer.nnewline);

                    // Wake up the processing waiting to read.
                    wakeup(&line_buffer);
                    continue;
                }

                // Backspace
                if (c == '\b' || c == 0x7f) {
                    if (line_buffer.len > 0) {
                        line_buffer.len--;
                        line_buffer.cursor--;
                    }
                    // Destructive backspace: Move back, overwrite with space, move back.
                    uart_putc(COM1, '\b');
                    uart_putc(COM1, ' ');
                    uart_putc(COM1, '\b');
                    continue;
                }

                // Normal Characters, insert and Echo
                if (c >= 0x20 && c < 0x7f) {
                    if (line_buffer.len < LINE_BUF_SZ - 1) {
                        line_buffer.data[line_buffer.cursor++] = c;
                        line_buffer.len++;
                        uart_putc(COM1, c);
                    }
                    continue;
                }
                break;

            case STATE_ESC:
                if (c == '[') {
                    line_buffer.state = STATE_ESC_BRACKET;
                } else {
                    line_buffer.state = STATE_NORMAL; // ignored
                }
                break;

            case STATE_ESC_BRACKET:
                switch (c) {
                    case 'D':
                        // Left Arrow
                        if (line_buffer.cursor > 0) {
                            line_buffer.cursor--;
                            uart_putc(COM1, '\b'); // Just move the visual cursor
                        }
                        break;
                    case 'C':
                        if (line_buffer.cursor < line_buffer.len) {
                            // Re-echo the char to move right
                            uart_putc(COM1, line_buffer.data[line_buffer.cursor]);
                            line_buffer.cursor++;
                        }
                        break;
                }
                line_buffer.state = STATE_NORMAL;
                break;
        }
    }

    unlock(&line_buffer.lk);
}



/* ------------------------------
 * Raw Mode
 */


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


/* Raw mode handler. When a byte arrives we simply put it on the buffer, then
 * wake up the sleeping process immediately.
 * */
void console_handler_raw() {
    char c;
    lock(&raw_buffer.lk);

    while ((c = uart_getc(COM1)) != -1) {
        if (c < -1) continue; // skip invalid char


        if (raw_buffer.nwritten == raw_buffer.nread) {
            raw_buffer.data[raw_buffer.nwritten++ % RAW_BUF_SZ] = c;

            // Wake up any pending read process.
            wakeup(&raw_buffer.nread);
        }
    }

    unlock(&raw_buffer.lk);
}


/* Write n bytes from addr to console. We simply write to the screen. */
int console_write(Inode *ino, char *addr, int n) {
    if (ino->d.type != F_DEV || ino->d.major != DEV_CONSOLE) {
        panic("wrong device\n");
    }

    for (int i = n; i > 0; i--) {
        putc(addr++);
    }

    return n;
}


/* ------------------------------
 * Interface
 */


int console_read(Inode *ino, char *addr, int n) {
    switch (console_mode) {
        case CONSOLE_COOKED_MODE: return console_read_cooked(ino, addr, n);
        case CONSOLE_RAW_MODE:    return console_read_raw(ino, addr, n);
    }
}


void console_handler() {
    switch (console_mode) {
        case CONSOLE_COOKED_MODE: console_handler_cooked(); break;
        case CONSOLE_RAW_MODE:    console_handler_raw(); break;
    }
}


void console_init() {
    console_mode = CONSOLE_COOKED_MODE;
    line_buffer.lk = new_lock("line_buffer");
    memset(line_buffer.data, 0, LINE_BUF_SZ);

    raw_buffer.lk = new_lock("raw_buffer");
    memset(raw_buffer.data, 0, RAW_BUF_SZ);

    devices[DEV_CONSOLE].read = console_read;
    devices[DEV_CONSOLE].write = console_write;
}
