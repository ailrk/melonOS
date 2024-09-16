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
#include "process/proc.h"

#define DBG 0

/* defined in `kernel.ld.
 *`first address after kernel loaded from ELF file
 */
extern char end[];

PDE *kernel_page_dir;     // kernel only page directory.
char *kstack;             // kernel stack. userd in entry.s

extern char data[];

void kmain(void) {
    tty_init();
    palloc_init(end, P2V_C(PTESZ * NPDES * NPTES));
    allocate_kernel_vmem();
    gdt_init();
    trap_init();
    pic_init();
    palloc_init(P2V_C(PTESZ * NPDES * NPTES), P2V_C(PHYSTOP));
    ps2_init();
    ptable_init();
    init_pid1();
    idt_init();
    scheduler();
}
