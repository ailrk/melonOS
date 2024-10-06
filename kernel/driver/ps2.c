#include <stdbool.h>
#include "ps2.h"
#include "i386.h"
#include "err.h"
#include "driver/vga.h"

#define DEBUG 0


/*! wait for the input buffer to be ready */
static void waiti() {
    int n = 0x100;
    while (n--) {
        if (inb((KBP_STAT & KBS_IB) == 0))
            return;
    }
}

/*! wait for the output buffer to be ready */
static void waito() {
    int n = 0x100;
    while (n--) {
        if (inb((KBP_STAT & KBS_OB) != 0))
            return;
    }
}

void ps2out(uint16_t port, uint8_t val) {
    waito();
    outb(port, val);
}

uint8_t ps2in(uint16_t port)  {
    waiti();
    return inb(port);
}


static void disable_p1() {
    ps2out(KBP_CMD, 0xad);
}

static void disable_p2() {
    ps2out(KBP_CMD, 0xa7);
}

static void enable_p1() {
    ps2out(KBP_CMD, 0xae);
}

static void enable_p2() {
    ps2out(KBP_CMD, 0xa8);
}

static bool self_test1() {
    ps2out(KBP_CMD, 0xaa);
    return ps2in(KBP_DATA) == 0x55;
}

static bool self_test2() {
    ps2out(KBP_CMD, 0xa9);
    return ps2in(KBP_DATA) == 0x00;
}

static void flush_datap() {
    ps2in(KBP_DATA);
}


/*! Read controller configuration bytes */
static uint8_t read_ccb() {
    ps2out(KBP_CMD, 0x20);
    return ps2in(KBP_DATA);
}

static void write_ccb(uint8_t b) {
    ps2out(KBP_CMD, 0x60);
    ps2out(KBP_DATA, b);
}


void ps2_reset() {
    uint8_t data;
    ps2out(KBP_DATA, 0xff);
    data = ps2in(KBP_DATA);
    if (data != 0xfa) {
        perror("[\033[31mkbd\033[0m] failed to reset\n");
        return;
    }

    data = ps2in(KBP_DATA);
    if (data != 0xaa) {
        perror("[\033[31mkbd\033[0m] failed to reset\n");
        return;
    }
}


static void echo_test() {
    ps2out(KBP_DATA, 0xee);
    if (ps2in(KBP_DATA) != 0xee) {
        perror("[\033[31mkbd\033[0m] failed to echo\n");
    }
}


/*! Scan code command. If `scancode` is 0, returns the current scan code. Otherwise
 *  set the scan code to `scancode`
 * */
static uint8_t scancode(uint8_t scancode) {
    if (scancode > 0x3) {
        perror("[\033[33mkbd\033[0m] invalid scan code");
        return 0;
    }
    uint8_t data;
    ps2out(KBP_DATA, 0xf0);
    data = ps2in(KBP_DATA);

    if (data != 0xfa && data != 0xfe) {
        perror("[\033[33mkbd\033[0m] can't set scan code");
        return 0;
    }

    ps2out(KBP_DATA, scancode);
    data = ps2in(KBP_DATA);

    if (data != 0xfa && data != 0xfe) {
        perror("[\033[33mkbd\033[0m] can't set scan code");
        return 0;
    }

    return ps2in(KBP_DATA);
}



#if DEBUG
static void debug() {
    vga_printf("[\033[33mkbd\033[0m] status %#x, data %#x\n", inb(KBP_STAT), inb(KBP_DATA));
}
#endif


/* controller configuration byte */
#define CCB_P1_INT  0 // port 1 interrupt 1=enable
#define CCB_P2_INT  1 // port 2 interrupt 1=enable
#define CCB_SYSFLAG 2 // 1=past POST
#define CCB_P1_CLK  4 // port 1 clock 1=disabled
#define CCB_P2_CLK  5 // port 2 clock 1=disbaled
#define CCB_P1_TRA  6 // port 1 translation 1=enabled


void ps2_init() {
    vga_printf("[\033[32mboot\033[0m] ps2...");

    echo_test();

    // disable both ports during initialization
    disable_p1();
    disable_p2();
    flush_datap();

    // disable port 1 interrupts
    write_ccb(read_ccb() & ~((1 << CCB_P1_INT) | (1 << CCB_P1_TRA)));

    // post test
    if (!self_test1()) {
        perror("[\033[31mkbd\033[0m] failed self test: second p/s2\n");
    }
    if (!self_test2()) {
        perror("[\033[31mkbd\033[0m] failed self test: second p/s2\n");
    }

    scancode(0x2);

    enable_p1();
    enable_p2();

    // enable port 1 interrupts
    write_ccb(read_ccb() | (1 << CCB_P1_INT) | (1 << CCB_P1_TRA));
    vga_printf("\033[32mok\033[0m\n");
}
