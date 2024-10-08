#pragma once

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
#define I_IRQ_CASCADE    0x02    // Enable slave pic
#define I_IRQ_COM2       0x03    // IRQ com2 serial port
#define I_IRQ_COM1       0x04    // IRQ com1 serial port
#define I_IRQ_LPT1       0x05    // IRQ parallel port
#define I_IRQ_CMOS       0x08    // IRQ cmos clock
#define I_IRQ_MOUSE      0x0C    // IRQ mouse
#define I_IRQ_IDE        0x0E    // IRQ ide device
#define I_IRQ_ERR        0x13    // IRQ error
#define I_IRQ_SPURIOUS   0x1F    // IRQ error

/* melonos specific vectors */
#define I_SYSCALL        0x40    // system call
#define I_DEFAULT        0xff    // catch all
