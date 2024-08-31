#include "tty.h"
#include "idt.h"
#include "i386.h"
#include "kbd.h"
#include "mmu.h"

PDE *page_dir;      // page directory for paging. initailized in entry.s
char *kstack;       // kernel stack. userd in entry.s

int kmain(void) {
    // idt_init();
    vga_tty_init();

    vga_tty_printf("kmain\n");
    __asm__ volatile ("cli; hlt");
    for (;;) {
    }

    return 0;
}
