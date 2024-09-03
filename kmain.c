#include "defs.h"
#include "mem.h"
#include "palloc.h"
#include "pic.h"
#include "tty.h"
#include "idt.h"
#include "i386.h"
#include "kbd.h"
#include "mmu.h"
#include "gdt.h"


/* defined in `kernel.ld.
 *`first address after kernel loaded from ELF file
 */
extern char *end;        

PDE *kernel_page_dir;     // kernel only page directory. 
char *kstack;             // kernel stack. userd in entry.s


int kmain(void) {
    vga_tty_init();
    palloc_init(0, P2V_C(PTESZ * NPDES * NPTES));
    pic_init();
    idt_init();
    gdt_init();
    palloc_init(P2V_C(PTESZ * NPDES * NPTES), (void*)PHYSTOP);

    __asm__ volatile ("cli; hlt");
    for (;;) {
    }

    return 0;
}
