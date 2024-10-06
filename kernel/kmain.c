#include "defs.h"
#include "dev.h"
#include "trap.h"
#include "memory.h"
#include "fs.h"
#include "driver/vga.h"
#include "driver/uart.h"
#include "process.h"

#define DBG 0

char kstack[KSTACK_SZ]; // kernel stack. userd in entry.s


void kmain(void) {
    vga_init();
    mem_init1();
    uart_init();
    mem_init2();
    trap_init();
    fs_init();
    dev_init();
    process_init();
    scheduler();
}
