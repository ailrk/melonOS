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

PDE *bootstrap_page_dir;  // page directory for paging. initailized in entry.s
char *kstack;             // kernel stack. userd in entry.s


int kmain(void) {
    palloc_init(0, P2V_C(4 * NPDES * NPTES));
    vga_tty_init();
    pic_init();
    idt_init();
    gdt_init();
    palloc_init(P2V_C(4 * NPDES * NPTES), (void*)PHYSTOP);

    __asm__ volatile ("cli; hlt");
    for (;;) {
    }

    return 0;
}
