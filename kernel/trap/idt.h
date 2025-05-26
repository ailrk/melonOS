#pragma once
#include <stdint.h>


#define IDT_MAX_VECTOR 256


typedef struct IDTEntry {
	uint16_t isr_low;    // lower 16 bits offset of isr address
	uint16_t kernel_cs;  // GDT segment selector, the CPU will load into CS before calling the ISR
	uint8_t  reserved;   // always set to zero
	uint8_t  attributes; // type and attributes; see the IDT page
	uint16_t isr_high;   // higher 16 bits of the isr address
} __attribute__((packed)) IDTEntry;


typedef struct IDTRecord {
    uint16_t  limit; // size of IDT array - 1
    uint32_t  base;  // pointer to IDT array
} __attribute__((packed)) IDTRecord;


void *get_handler_from_idt(uint8_t vector);
void  regist_idt_handler(uint8_t vector, void *isr, uint8_t flags);
void  idt_init();
