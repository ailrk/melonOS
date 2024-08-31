#include "tty.h"
#include "idt.h"
#include "i386.h"
#include "kbd.h"
#include "mmu.h"

extern char kbegin[];     // first address after kernel loaded from ELF file
PDE *bootstrap_page_dir;  // page directory for paging. initailized in entry.s
char *kstack;             // kernel stack. userd in entry.s

extern PDE *kpgdir;
int kmain(void) {
    idt_init();
    vga_tty_init();

    __asm__ volatile ("cli; hlt");
    for (;;) {
    }

    return 0;
}
