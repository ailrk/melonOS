#include "idt.h"
#include <stdint.h>

// an array of IDT entries; 
IDTEntry idt[IDT_MAX_VECTOR];


// IDT pointer for `lidt`
IDTRecord idtr;

// indicate available vectors.
int vectors[IDT_MAX_VECTOR];


// default exception handler 
__attribute__((noreturn))
void exception_handler() {
    __asm__ volatile ("cli; hlt"); // hangs the computer
    //__asm__ volatile ("iret"); // hangs the computer
}

// reconstruct the handler ptr from the IDT table. 
IDTHanlder get_handler_from_idt(uint8_t vector) {
    IDTEntry e = idt[vector];
    uint32_t high = e.isr_high << 16;
    uint32_t low = e.isr_low;
    uint32_t ptr = high | low;
    return (IDTHanlder)ptr;
}


void regist_idt_handler(uint8_t vector, void *isr, uint8_t flags) {
    IDTEntry *entry = &idt[vector];
    entry->isr_low = (uint32_t) isr & 0xffff;
    entry->kernel_cs = 0x08; // 8 bytes after gdt 
    entry->reserved = 0;
    entry->attributes = flags;
    entry->isr_high = (uint32_t) isr >> 16;
}

void load_idt() {
    idtr.base = (uint32_t)&idt;
    idtr.limit = sizeof(IDTEntry) * IDT_MAX_VECTOR - 1;
    for (uint16_t i = 0; i < IDT_MAX_VECTOR; ++i) {
        regist_idt_handler(i, &exception_handler, InterruptGate);
    }

    regist_idt_handler(I_SYSCALL, &exception_handler, TrapGate);
    
    lidt((void*)&idtr);
    sti();
}
