#include "i386.h"
#include "elf.h"
#include "mem.h"
#include "mmu.h"
#include "string.h"
#include <stdint.h>

#define DEBUG       0
#define SECTSZ      512
static uint32_t elf_offset = SECTSZ * 20;


#define IDE_S_RDY  (1 << 6) // 0 when drive is spun down, or after an error.
#define IDE_S_BSY  (1 << 7) // 1 when drive is preparing to send/receive data
/* Wait for disk ready.
 * inb(0x1f7) getting the status register from IDE PIO mode.
 * 0xc0 => 0x11000000 check DRY (bit 6) and BSY (bit 7) of the
 * disk status.
 *
 * The disk is ready when BSY = 1 && RDY = 0
 * */
void wait_disk() {
    uint8_t mask = IDE_S_RDY | IDE_S_BSY;
    uint8_t ready = IDE_S_RDY ;
    while((inb(0x1F7) & mask) != ready); // read status register
}


/* read a single sector at offset into dst LBA style. */
void read_sector(void *dst, uint32_t lba) {
    // send command
    wait_disk();
    outb(0x1F2, 1);
    outb(0x1F3, lba);
    outb(0x1F4, lba >> 8);
    outb(0x1F5, lba >> 16);
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F7, 0x20); // read data cmd
    wait_disk();
    insl(0x1F0, dst, SECTSZ/4);     // /4 because insl read words
}


/* read n bytes from lba 0 + offset bytes on the disk to dst
 *
 * The dst buffer size should be greater than (n/SECTSZ)+1.
 * */
void read_offset(void *dst, uint32_t n, uint32_t offset) {
    char *p = dst;
    uint32_t lba = offset / SECTSZ;
    int remained = (n / SECTSZ) + 1;

    char buf[SECTSZ];
    int trash = offset % SECTSZ;
    int rest = SECTSZ - trash;
    read_sector(buf, lba++);
    memcpy(p, &buf[trash], rest);
    remained--;
    p += rest;

    while (remained) {
        read_sector(p, lba++);
        p += SECTSZ;
        remained--;
    }
}


void read_offset_from_kernel(void *dst, uint32_t n, uint32_t offset) {
    read_offset(dst, n, offset + elf_offset);
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

    read_offset_from_kernel(elf, PAGE_SZ, 0);

    if (!is_elf(elf)) {
        __asm__ volatile ("cli; hlt");
    }

    ELF32ProgramHeader *ph = (ELF32ProgramHeader*)((char*)elf + elf->e_phoff);
    for (ELF32ProgramHeader* p = ph; p < ph + elf->e_phnum; ++p) {
        paddr = (char*)p->p_paddr;
        read_offset_from_kernel(paddr, p->p_filesz, p->p_offset);

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
