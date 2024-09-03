#include "gdt.h"
#include "i386.h"
#include "mmu.h"
#include "proc.h"
#include "tty.h"


extern char data[]; // defined by kernel.ld
PDE *kpage_dir;     // for scheduler


extern CPU cpu;

void gdt_init() {
    tty_printf("[boot] gdt...");
    cpu.gdtr.base = (uint32_t)&cpu.gdt;
    cpu.gdtr.limit = sizeof(GDTEntry) * NSEGS - 1;
    cpu.gdt[SEG_KCODE] = create_segment_entry(0, 0xffffffff, ST_X | ST_R, 0);
    cpu.gdt[SEG_KDATA] = create_segment_entry(0, 0xffffffff, ST_W, 0);
    cpu.gdt[SEG_UCODE] = create_segment_entry(0, 0xffffffff, ST_X | ST_R, DPL_USER);
    cpu.gdt[SEG_UDATA] = create_segment_entry(0, 0xffffffff, ST_W, DPL_USER);
    lgdt((void*)&cpu.gdtr);
    tty_printf("ok\n");
}
