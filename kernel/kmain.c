#include "memory/palloc.h"
#include "trap.h"
#include "memory.h"
#include "fs.h"
#include "driver/vga.h"
#include "driver/ps2.h"
#include "driver/uart.h"
#include "process/proc.h"

#define DBG 0


extern char end[];  // defined in `kernel.ld.
extern char data[]; // elf segment
char *      kstack; // kernel stack. userd in entry.s


void kmain(void) {
    vga_init();
    palloc_init(end, P2V_C(PTESZ * NPDES * NPTES));
    mem_init1();
    uart_init();
    ptable_init();
    trap_init();
    ps2_init();
    mem_init2();
    fs_init();
    init_pid1();
    scheduler();
}
