#include "defs.h"
#include "i386.h"
#include "elf.h"
#include "mem.h"
#include "mmu.h"
#include "ide.h"
#include <stdalign.h>
#include <stdint.h>

#define DEBUG 0

#if DEBUG
/* simple print util for debugging */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
uint16_t *vgabuf = (uint16_t *)0xb8000; int x = 0; int y = 0;
void putc(char c) {
    vgabuf[y * VGA_WIDTH + x] = c | (15 << 8);
    if (x == VGA_WIDTH) { x=0; y= (y+1) % VGA_HEIGHT; } else x++;
}
void puts(char* c) { while(*c) putc(*c++); }
void phex(uint32_t n) {
    static const char digits[] = "0123456789ABCDEF";
    char buf[32]; int i = 0; int base = 16;
    do { buf[i++] = digits[n % base]; n /= base; } while(n);
    buf[i++] = 'x'; buf[i++] = 'b'; buf[i++] = '\0';
    puts(strrev(buf));
}
#endif


static uint32_t elf_offset = SECTSZ * 20;

void ata_read_offset_from_kernel(void *dst, uint32_t n, uint32_t offset) {
    ata_read_offset(dst, n, offset + elf_offset);
}


PDE pagedir[NPDES] __attribute__((aligned(PAGE_SZ)));
PDE pagetbl[NPTES] __attribute__((aligned(PAGE_SZ)));


/*! The boot loader setup a temporary virtual memory so we can use virtual address.
 *  The kernel will switch to a different page table.
 *
 *  Only setup 1 page table which gives 4MB virtual memory.
 * */
void paging_init() {
    for (int i = 0; i < NPDES; ++i) pagedir[i] = 0 | PDE_W;

    // page table for PA [0, 4096 * 1024)
    for (int i = 0; i < NPTES; ++i) pagetbl[i] = (i * 0x1000) | PTE_P | PTE_W;

    // map VA [0, 4MB) to PA [0, 4MB)
    pagedir[0] = (uint32_t)pagetbl | PDE_P | PDE_W; 

    // map VA [KERN_BASE, KERN_BASE+4MB) to PA [0, 4MB)
    pagedir[KERN_BASE>>PD_IDX_SHIFT] = (uint32_t)pagetbl | PDE_P | PDE_W; 

    set_cr3((physical_addr)pagedir);
    set_cr0(get_cr0() | CR0_WP | CR0_PG);
}



/* boot3 of the boot sequence 
 * - loads the kernel elf image from the disk sector 1
 * - jump to the entry (kmain)
 * */

void boot3() {
    char *paddr;

    // put elf at some unused space. After kernel is loaded
    // this memory is not used.
    ELF32Header *elf = (ELF32Header *)0x10000;
    
    ata_read_offset_from_kernel(elf, PAGE_SZ, 0);

    if (!is_elf(elf)) {
        __asm__ volatile ("cli; hlt");
    }

    ELF32ProgramHeader *ph = (ELF32ProgramHeader*)((char*)elf + elf->e_phoff);
    for (ELF32ProgramHeader* p = ph; p < ph + elf->e_phnum; ++p) {
        paddr = (char*)p->p_paddr;
        ata_read_offset_from_kernel(paddr, p->p_filesz, p->p_offset);

        // pad till memsz
        if (p->p_memsz > p->p_filesz)
            stosb(paddr + p->p_filesz, 0, p->p_memsz - p->p_filesz);
    }

    paging_init();

    void(*entry)();
    entry = (void(*)(void))(V2P_C(elf->e_entry));
    entry();
    __asm__ volatile ("cli; hlt");
}
