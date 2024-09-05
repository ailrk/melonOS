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


typedef struct IDTRecord {
    uint16_t  limit;      // size of IDT array - 1
    uint32_t  base;       // pointer to IDT array
} __attribute__((packed)) IDTRecord;


// 8 bytes flag that describes the idt
// | 7 | 6-5 | 4 |  3-0      |
// | p | dpl | 0 | gate type |
enum IDTFlags {
    InterruptGate = 0x8E,   // handle interrupt
    TrapGate = 0x8F,        // handle trap
    TaskGate = 0x85         // for hardware task switching
};


void *get_handler_from_idt(uint8_t vector);
void regist_idt_handler(uint8_t vector, void *isr, uint8_t flags);
void idt_init();


/* IRQ is the device interrupt that comes from PIC/APIC. 
 * APIC will forward it to the LAPIC which is the CPU interrupt.
 */


/* CPU exceptions */
#define I_DIVBYZERO      0x00   // devide by zero
#define I_DEBUG          0x01   // debug interrupt
#define I_NMI            0x02   // non maskable interrupt
#define I_BRKPNT         0x03   // breakpoint
#define I_OVERFLOW       0x04   // overflow
#define I_BOUND          0x05   // boud check exceed
#define I_ILLEGALOP      0x06   // ilegal op
#define I_COPNOAVIL      0x07   // coprocessor not available
#define I_DOUBLEFLT      0x08   // double fault
#define I_COPSEG         0x09   // coprocessor sement overrun
#define I_TSS            0x0A   // invalid tss 
#define I_SEGNP          0x0B   // segment not present
#define I_STKSGFLT       0x0C   // stack segment fault
#define I_GPFLT          0x0D   // general protection fault
#define I_PGFLT          0x0E   // page fault
                                //  15 is reserved
#define I_FPERR          0x10   // floating point error 
#define I_ALIGN          0x11   // alignment check 
#define I_MACHINE        0x12   // machine check
#define I_SIMDERR        0x13   // simd floating point error

/* hardware interrupts
 *
 * by default PIC/APIC IRQ overlaps with the default CPU interrupt vectors,
 * a common way to handle this is to remap them to some unused vectors.  
 *
 * Usually IRQ0 goes to 0x20 (32).
 * */
#define IRQ0             0x20
#define MAP_IRQ(irq_line)        (IRQ0 + irq_line)

#define I_IRQ_TIMER      0x00    // IRQ timer
#define I_IRQ_KBD        0x01    // IRQ keyboard
                                 // 0x02 is used internally by pic chips
#define I_IRQ_COM1       0x04    // IRQ com1 serial port
#define I_IRQ_IDE        0x0E    // IRQ ide device
#define I_IRQ_ERR        0x13    // IRQ error
#define I_IRQ_SPURIOUS   0x1F    // IRQ error

/* melonos specific vectors */
#define I_SYSCALL        0x40    // system call 
#define I_DEFAULT        0xff    // catch all
