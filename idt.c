#include "idt.h"

// an array of IDT entries; 
static IDTEntry idt[IDT_MAX_VECTOR]; 


// IDT pointer for `lidt`
static IDTRecord idtr;

// indicate available vectors.
static int vectors[IDT_MAX_VECTOR];

// default exception handler 
__attribute__((noreturn))
void exception_handler() {
    __asm__ volatile ("cli; hlt"); // hangs the computer
}



void regist_idt_handler(uint8_t vector, void *isr, uint8_t flags) {
    IDTEntry *entry = &idt[vector];
    entry->isr_low = (uint32_t) isr & 0xffff;
    entry->kernel_cs = 0x08; // 8 bytes after gdt 
    entry->reserved = 0;
    entry->attributes = flags;
    entry->isr_high = (uint32_t) isr >> 16;
}

static void* isr_handler_table[IDT_MAX_VECTOR];


void load_idt() {
    idtr.base = (uintptr_t)idt;
    idtr.limit = sizeof(IDTEntry) * IDT_MAX_VECTOR - 1;

    for (uint16_t i = 0; i < IDT_MAX_VECTOR; ++i) {
        regist_idt_handler(i, isr_handler_table[i], InterruptGate);
    }

    regist_idt_handler(I_SYSCALL, isr_handler_table[I_SYSCALL], TrapGate);

    
    lidt((void*)&idtr);
    sti();
}
