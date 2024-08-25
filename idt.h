#pragma once

#include <stdint.h>
#include "i386.h"

#define IDT_MAX_VECTOR 256

typedef struct IDTEntry {
	uint16_t isr_low;      // lower 16 bits offset of isr address
	uint16_t kernel_cs;    // GDT segment selector, the CPU will load into CS before calling the ISR
	uint8_t  reserved;     // always set to zero
	uint8_t  attributes;   // type and attributes; see the IDT page
	uint16_t isr_high;     // higher 16 bits of the isr address
} __attribute__((packed)) IDTEntry;


typedef struct IDTRecord
{
    uint16_t  limit;      // size of IDT array - 1
    uint32_t  base;       // pointer to IDT array
} __attribute__((packed)) IDTRecord ;


typedef void (*IDTHanlder)();

// 8 bytes flag that describes the idt
// | 7 | 6-5 | 4 |  3-0      |
// | p | dpl | 0 | gate type |
enum IDTFlags {
    InterruptGate = 0x8E,   // handle interrupt
    TrapGate = 0x8F,        // handle trap
    TaskGate = 0x85         // for hardware task switching
};


void exception_handler();
IDTHanlder get_handler_from_idt(uint8_t vector);
void regist_idt_handler(uint8_t vector, void *isr, uint8_t flags);
void load_idt();


#define IRQ0        32
enum {
    /* processor defined */
    I_DIVBYZERO    =  0,   // devide by zero
    I_DEBUG        =  1,   // debug interrupt
    I_NMI          =  2,   // non maskable interrupt
    I_BRKPNT       =  3,   // breakpoint
    I_OVERFLOW     =  4,   // overflow
    I_BOUND        =  5,   // boud check exceed
    I_ILLEGALOP    =  6,   // ilegal op
    I_COPNOAVIL    =  7,   // coprocessor not available
    I_DOUBLEFLT    =  8,   // double fault
    I_COPSEG       =  9,   // coprocessor sement overrun
    I_TSS          = 10,   // invalid tss 
    I_SEGNP        = 11,   // segment not present
    I_STKSGFLT     = 12,   // stuck segment fault
    I_GPFLT        = 13,   // general protection fault
    I_PGFLT        = 14,   // page fault
                           // reserved
    I_FPERR        = 15,   // floating point error 
    I_ALIGN        = 16,   // alignment check 
    I_MACHINE      = 17,   // machine check
    I_SIMDERR      = 18,   // simd floating point error
    I_VIRERR       = 19,   // virtualization error
    
    I_IRQ_TIMER    = IRQ0,        // IRQ timer
    I_IRQ_KBD      = IRQ0 + 1,    // IRQ keyboard
    I_IRQ_COM1     = IRQ0 + 4,    // IRQ com1 serial port
    I_IRQ_IDE      = IRQ0 + 14,   // IRQ ide device
    I_IRQ_ERR      = IRQ0 + 19,   // IRQ error
    I_IRQ_SPURIOUS = IRQ0 + 31,   // IRQ error

    /* melonos specific vectors */
    I_SYSCALL    =  64,   // system call 
    I_DEFAULT    = 300,   // catch all
};
