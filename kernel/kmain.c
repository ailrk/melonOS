#include "defs.h"
#include "dev.h"
#include "trap.h"
#include "memory.h"
#include "fs.h"
#include "process.h"
#include "driver/vga.h"
#include "driver/uart.h"

char kstack[KSTACK_SZ]; // kernel stack. userd in entry.s


void kmain (void) {
    vga_init();
    mem_init();
    uart_init();
    trap_init();
    dev_init();
    fs_init();
    process_init();
    scheduler();
}
