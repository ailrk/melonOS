#include "idt.h"
#include "isr.h"
#include "pic.h"
#include <stdint.h>

/* an array of IDT entries; */
IDTEntry idt[IDT_MAX_VECTOR];


/* IDT pointer for `lidt` */
IDTRecord idtr;

/* indicate available vectors. */
int vectors[IDT_MAX_VECTOR];


/* reconstruct the handler ptr from the IDT table. */
void *get_handler_from_idt(uint8_t vector) {
    IDTEntry e = idt[vector];
    uint32_t high = e.isr_high << 16;
    uint32_t low = e.isr_low;
    uint32_t ptr = high | low;
    return (void*)ptr;
}


void regist_idt_handler(uint8_t vector, void *isr, uint8_t flags) {
    IDTEntry *entry = &idt[vector];
    entry->isr_low = (uint32_t) isr & 0xffff;
    entry->kernel_cs = 0x08; // 8 bytes after gdt 
    entry->reserved = 0;
    entry->attributes = flags;
    entry->isr_high = (uint32_t) isr >> 16;
}

void idt_init() {
    idtr.base = (uint32_t)&idt;
    idtr.limit = sizeof(IDTEntry) * IDT_MAX_VECTOR - 1;
    isr_register();
    lidt((void*)&idtr);
    sti();
}
