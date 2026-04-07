#include "i386.h"
#include "mmu.h"
#include "print.h"
#include "memory/gdt.h"
#include "process/pdefs.h"

/* GDT is not useful if you have paging, but x86 protected mode requires you
 * to setup one.
 *
 * When a X86 instruction tries to access a piece of memory, it will check if
 * that memory has the privilege to be access. e.g read/write/execution privilege.
 * The privilege was historically set with gdt with segmented memory model,
 * you used to have CS (code segment), DS (data segment), SS (stack segment) and
 * so on. Each segment has its own privilege.
 *
 * If we use paging, there is only one segment: the entire flat memory. We
 * simply map all segments to the entire memory, this is the Flat Memory model.
 * But we still need to set the privilege for the segment so instructions know
 * it can access the memory. We do that by reusing the gdt facility.
 *
 * This is why we need to set 4 different segment descriptors and they all cover
 * the full range of the memory. KCODE and KDATA allows the entire memory range
 * to be read/wrote/executed, so does UCODE and UDATA.
 *
 * The difference between K/U is the CPL-a global privilege level that acts as a
 * "mode" for the cpu. If CPL is at ring 3, it's in user mode, otherwise it's in
 * kernel mode. In paging, the memory range doesn't matter for privilege mode.
 * Each mapped page has a corresponding page table entry in which there is a bit
 * that indicates it's privilege.
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
    dprintf(LOG_BOOT " gdt...");
    cpu.gdtr.limit     = sizeof(GDTEntry) * NSEGS - 1;
    cpu.gdtr.base      = (uint32_t)&cpu.gdt;

    cpu.gdt[SEG_NULL]  = create_descriptor(0, 0, 0);

    // 0x08 Kernel code segment
    {
        uint16_t flag = SEG_S(1)       | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)       | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_K) | SEG_CODE_EXRD;
        cpu.gdt[SEG_KCODE] = create_descriptor (0, 0xffffffff, flag);
    }

    // 0x10 Kernel data segment
    {
        uint16_t flag = SEG_S(1)       | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)       | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_K) | SEG_DATA_RDWR;
        cpu.gdt[SEG_KDATA] = create_descriptor (0, 0xffffffff, flag);
    }

    // 0x18 User code segment
    {
        uint16_t flag = SEG_S(1)        | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)        | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_U)  | SEG_CODE_EXRD;
        cpu.gdt[SEG_UCODE] = create_descriptor (0, 0xffffffff, flag);
    }

    // 0x20 User data segment
    {
        uint16_t flag = SEG_S(1)       | SEG_P(1)  | SEG_AVL(0) |
                        SEG_L(0)       | SEG_DB(1) | SEG_G(1)   |
                        SEG_DPL(DPL_U) | SEG_DATA_RDWR;
        cpu.gdt[SEG_UDATA] = create_descriptor (0, 0xffffffff, flag);
    }

    // Load gdt
    // The CPU has hidden internal registers that cache the gdt, so
    // it doesn't have to read the gdt in memory everytime.
    //
    // Calling lgdt only sets the new gdp table, it does not change
    // the internal shadow registers yet. We need to load the new
    // segments and ljump.
    lgdt ((void*)&cpu.gdtr);


    // Force the CPU to reload the segments from the NEW table
    // Start with KCODE and KDATA so we are in kernel mode.
    // This avoids problem from having staled setup inherited
    // from the bootloader.
    __asm__ volatile (
        "movw $0x10, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
        : : : "ax", "memory"
    );

    // Far Jump to flush the pipeline and update CS
    //
    // The two least significant bits of the $CS register defines
    // CPL. You cannot mannually set it otherwise you can change
    // the privilege whenever you want.
    //
    // ljump takes a seletor, here 0x08, and force it into $CS.
    // $1f is the value forced into EIP, which is the next line.
    __asm__ volatile (
        "ljmp $0x08, $1f\n"
        "1:\n"
    );

    // At this point we basically created a static GDT, starts from
    // kernel mode.
    //
    // To switch tasks, we still need SEG_TSS entry. SEG_TSS segment
    // needs to know where the kernel stack is.
    //
    // TSS is like a static trapdoor, but because we have processes,
    // each process has its own kernel stack. We have one trapdoor
    // but mutliple stackes. the address needs to be set dynamically.
    //
    // Everytime we switch to user mode, we need to update the TSS
    // segment and indicate the kernel stack address.
    dprintf(LOG_OK "\n");
}
