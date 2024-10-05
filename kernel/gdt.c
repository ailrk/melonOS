#include "gdt.h"
#include "i386.h"
#include "mmu.h"
#include "drivers/vga.h"
#include "process/proc.h"

/* GDT is not useful if you have paging, but x86 protected mode requires you
 * to setup one. We simply map all segments to the entire memory.
 * */


extern char data[]; // defined by kernel.ld
extern CPU  cpu;


GDTEntry create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
    GDTEntry descriptor;

    // Create the high 32 bit segment
    descriptor  =  limit       & 0x000f0000;         // set limit bits 19:16
    descriptor |= (flag <<  8) & 0x00f0ff00;         // set type, p, dpl, s, g, d/b, l and avl fields
    descriptor |= (base >> 16) & 0x000000ff;         // set base bits 23:16
    descriptor |=  base        & 0xff000000;         // set base bits 31:24

    // Shift by 32 to allow for low part of segment
    descriptor <<= 32;

    // Create the low 32 bit segment
    descriptor |= base  << 16;                       // set base bits 15:0
    descriptor |= limit  & 0x0000ffff;               // set limit bits 15:0
    return descriptor;
}


void gdt_init() {
    vga_printf("[\033[32mboot\033[0m] gdt...");
    cpu.gdtr.limit     = sizeof(GDTEntry) * NSEGS - 1;
    cpu.gdtr.base      = (uint32_t)&cpu.gdt;

    cpu.gdt[SEG_NULL]  = create_descriptor(0, 0, 0);

    {
        uint16_t flag = SEG_S(1)       | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)       | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_K) | SEG_CODE_EXRD;
        cpu.gdt[SEG_KCODE] = create_descriptor(0, 0xffffffff, flag);
    }

    {
        uint16_t flag = SEG_S(1)       | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)       | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_K) | SEG_DATA_RDWR;
        cpu.gdt[SEG_KDATA] = create_descriptor(0, 0xffffffff, flag);
    }

    {
        uint16_t flag = SEG_S(1)        | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)        | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_U)  | SEG_CODE_EXRD;
        cpu.gdt[SEG_UCODE] = create_descriptor(0, 0xffffffff, flag);
    }

    {
        uint16_t flag = SEG_S(1)       | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)       | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_U) | SEG_DATA_RDWR;
        cpu.gdt[SEG_UDATA] = create_descriptor(0, 0xffffffff, flag);
    }

    lgdt((void*)&cpu.gdtr);
    vga_printf("\033[32moki\033[0m\n");
}
