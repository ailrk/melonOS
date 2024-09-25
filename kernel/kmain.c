#include "mem.h"
#include "trap.h"
#include "tty.h"
#include "idt.h"
#include "mmu.h"
#include "gdt.h"
#include "mem/vmem.h"
#include "mem/palloc.h"
#include "drivers/ps2.h"
#include "drivers/pic.h"
#include "drivers/uart.h"
#include "process/proc.h"
#include "fs/buffer.h"
#include "fs/disk.h"
#include "fs/file.h"

#define DBG 0

/* defined in `kernel.ld.
 *`first address after kernel loaded from ELF file
 */
extern char end[];

char *kstack;             // kernel stack. userd in entry.s

extern char data[];

void kmain(void) {
    tty_init();
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
