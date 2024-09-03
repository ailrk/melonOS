#include "i386.h"
#include "elf.h"
#include "mem.h"
#include "mmu.h"
#include "ata.h"
#include "ctrlregs.h"
#include <stdalign.h>
#include <stdint.h>

static uint32_t elf_offset = SECTSZ * 20;

void ata_read_offset_from_kernel(void *dst, uint32_t n, uint32_t offset) {
    ata_read_offset(dst, n, offset + elf_offset);
}


PDE pagedir[NPDES] __attribute__((aligned(PAGE_SZ)));
PDE pagetbl[NPTES] __attribute__((aligned(PAGE_SZ)));


void enable_paging(PDE *p) {
    set_cr3(p);
    set_cr0(get_cr0() | CR0_WP | CR0_PG);
}


/*! The boot loader setup a temporary virtual memory so we can use virtual address.
 *  The kernel will switch to a different page table.
 *
 *  Only setup a single page table which gives 4MB virtual memory.
 * */
void pagedir_init() {
    for (int i = 0; i < NPDES; ++i) pagedir[i] = 0 | PDE_W;
    for (int i = 0; i < NPTES; ++i) pagetbl[i] = (i * 0x1000) | PTE_P | PTE_W;
    pagedir[0] = (uint32_t)pagetbl| PDE_P | PDE_W; 
    pagedir[KERN_BASE>>PD_IDX_SHIFT] = (uint32_t)pagetbl | PDE_P | PDE_W; 

    enable_paging(pagedir);
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
    // read_sector_n((char*)elf, PAGE_SZ, 80);
    
    ata_read_offset_from_kernel(elf, PAGE_SZ, 0);

    if (!is_elf(elf)) {
        __asm__ volatile ("cli; hlt");
    }

    ELF32ProgramHeader* ph = (ELF32ProgramHeader*)((char*)elf + elf->e_phoff);
    for (ELF32ProgramHeader* p = ph; p < ph + elf->e_phnum; ++p) {
        paddr = (char*)p->p_paddr;
        ata_read_offset_from_kernel(paddr, ph->p_filesz, ph->p_offset);

        // pad till memsz
        if (ph->p_memsz > ph->p_filesz)
            stosb(paddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }

    pagedir_init();

    void(*entry)();
    entry = (void(*)(void))(V2P_C(elf->e_entry));
    entry();
    __asm__ volatile ("cli; hlt");
}
