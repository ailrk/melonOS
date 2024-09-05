#include "isr.h"
#include "mem.h"
#include "palloc.h"
#include "pic.h"
#include "tty.h"
#include "idt.h"
#include "mmu.h"
#include "gdt.h"
#include "vmem.h"


/* defined in `kernel.ld.
 *`first address after kernel loaded from ELF file
 */
extern char end[];        

PDE *kernel_page_dir;     // kernel only page directory. 
char *kstack;             // kernel stack. userd in entry.s


int kmain(void) {
    tty_init();
    palloc_init(end, P2V_C(PTESZ * NPDES * NPTES));
    kernel_vmem_alloc();
    gdt_init();
    pic_init();
    idt_init();
    palloc_init(P2V_C(PTESZ * NPDES * NPTES), P2V_C(PHYSTOP));

    for (;;) {
    }

    return 0;
}
