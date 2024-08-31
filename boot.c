#include "i386.h"
#include "elf.h"
#include "mem.h"
#include "tty.h"
#include "ata.h"
#include <stdalign.h>
#include <stdint.h>



static uint32_t elf_offset = SECTSZ * 80; 

void read_offset_from_kernel(void *dst, uint32_t n, uint32_t offset) {
    read_offset(dst, n, offset + elf_offset);
}


/* boot3 of the boot sequence 
 * - loads the kernel elf image from the disk sector 1
 * - jump to the entry (kmain)
 * */

void boot3() {
    vga_tty_init();
    vga_tty_printf("[boot3] reading kernel elf...\n");
    char *paddr;

    // put elf at some unused space. After kernel is loaded
    // this memory is not used.
    ELF32Header *elf = (ELF32Header *)0x10000;
    // read_sector_n((char*)elf, PAGE_SZ, 80);
    
    read_offset_from_kernel(elf, PAGE_SZ, 0);

    vga_tty_printf("ELF:\n");
    vga_tty_printf("%x,", elf->e_ident[0]);
    vga_tty_printf("%x,", elf->e_ident[1]);
    vga_tty_printf("%x,", elf->e_ident[2]);
    vga_tty_printf("%x,", elf->e_ident[3]);
    vga_tty_printf("%x,", elf->e_ident[4]);
    vga_tty_printf("%x,", elf->e_ident[5]);
    vga_tty_printf("%x,", elf->e_ident[6]);
    vga_tty_printf("%x,", elf->e_ident[7]);
    vga_tty_printf("%x,", elf->e_ident[8]);
    vga_tty_printf("%x,", elf->e_ident[9]);
    vga_tty_printf("%x,", elf->e_ident[10]);
    vga_tty_printf("%x,", elf->e_ident[11]);
    vga_tty_printf("%x,", elf->e_ident[12]);
    vga_tty_printf("%x,", elf->e_ident[13]);
    vga_tty_printf("%x,", elf->e_ident[14]);
    vga_tty_printf("%x,", elf->e_ident[15]);
    vga_tty_printf("%x,", elf->e_type);
    vga_tty_printf("%x,", elf->e_machine);
    vga_tty_printf("%x,", elf->e_version);
    vga_tty_printf("%x,", elf->e_entry);
    vga_tty_printf("%x,", elf->e_phoff);
    vga_tty_printf("%x,", elf->e_ehoff);
    vga_tty_printf("%x,", elf->e_flags);
    vga_tty_printf("%x,", elf->e_ehsize);
    vga_tty_printf("%x,", elf->e_ephsize);
    vga_tty_printf("%x,", elf->e_phnum);
    vga_tty_printf("%x,", elf->e_shentsize);
    vga_tty_printf("%x ", elf->e_shnum);
    vga_tty_printf("END ELF\n");

    if (!is_elf(elf)) {
        vga_tty_printf("not valid elf");
        __asm__ volatile ("cli; hlt");
    }

    ELF32ProgramHeader* ph = (ELF32ProgramHeader*)((char*)elf + elf->e_phoff);

    vga_tty_printf("PH :\n");
    vga_tty_printf("%x,", ph->p_type);
    vga_tty_printf("%x,", ph->p_offset);
    vga_tty_printf("%x,", ph->p_vaddr);
    vga_tty_printf("%x,", ph->p_paddr);
    vga_tty_printf("%x,", ph->p_filesz);
    vga_tty_printf("%x,", ph->p_memsz);
    vga_tty_printf("%x,", ph->p_flags);
    vga_tty_printf("%x,", ph->p_align);
    vga_tty_printf("END PH\n");

    for (ELF32ProgramHeader* p = ph; p < ph + elf->e_phnum; ++p) {
        paddr = (char*)p->p_paddr;
        read_offset_from_kernel(paddr, ph->p_filesz, ph->p_offset);

        // pad till memsz
        if (ph->p_memsz > ph->p_filesz)
            stosb(paddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
    }

    void(*entry)();
    entry = (void(*)(void))(V2P_C(elf->e_entry));
    vga_tty_printf("[boot3] jumping to the entry at paddr %p...\n", entry);
    entry();
    vga_tty_printf("should not run");
}
