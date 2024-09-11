#include "kbd.h"
#include "mem.h"
#include "proc.h"
#include "ps2.h"
#include "palloc.h"
#include "pic.h"
#include "trap.h"
#include "tty.h"
#include "idt.h"
#include "mmu.h"
#include "gdt.h"
#include "uart.h"
#include "vmem.h"

#define DBG 0

/* defined in `kernel.ld.
 *`first address after kernel loaded from ELF file
 */
extern char end[];        

PDE *kernel_page_dir;     // kernel only page directory. 
char *kstack;             // kernel stack. userd in entry.s

extern char data[];

int kmain(void) {
    tty_init();
    palloc_init(end, P2V_C(PTESZ * NPDES * NPTES));
    allocate_kernel_vmem();
    gdt_init();
    trap_init();
    pic_init();
    palloc_init(P2V_C(PTESZ * NPDES * NPTES), P2V_C(PHYSTOP));
    ps2_init();
    idt_init();
    init_pid1();
    uart_putc('a');
    for(;;);
    return 0;
}
