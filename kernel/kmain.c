#include "mem.h"
#include "trap.h"
#include "idt.h"
#include "mmu.h"
#include "gdt.h"
#include "mem/vmem.h"
#include "mem/palloc.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "drivers/pic.h"
#include "drivers/uart.h"
#include "process/proc.h"
#include "fs/bcache.h"
#include "fs/disk.h"
#include "fs/file.h"

#define DBG 0


extern char end[];  // defined in `kernel.ld.
extern char data[]; // elf segment
char *      kstack; // kernel stack. userd in entry.s


void kmain(void) {
    vga_init();
    palloc_init(end, P2V_C(PTESZ * NPDES * NPTES));
    kernel_vmem_init();
    gdt_init();
    trap_init();
    pic_init();
    uart_init();
    ps2_init();
    ptable_init();
    bcache_init();
    ftable_init();
    palloc_init(P2V_C(PTESZ * NPDES * NPTES), P2V_C(PHYSTOP));
    bcache_init();
    disk_init();
    idt_init();
    init_pid1();
    scheduler();
}
