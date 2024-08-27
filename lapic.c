// Local APIC manages non-I/O interrupts.


#include "lapic.h"
#include <stdint.h>
#include <stdbool.h>


/* definitions of lapic registers.
 * lapic registers are memory mapped to MADT (multiple apic description table).
 *
 * reference:
 * https://wiki.osdev.org/APIC#:~:text=use%20the%20attack.-,Local%20APIC%20registers,-The%20local%20APIC
 * */
#define ID      0x20  // ID
#define VER     0x30  // version
#define TPR     0x80  // task priority register
#define APR     0x90  // arbitrary priority register
#define PPP     0xA0  // process priority register
#define EOI     0xB0  // EOI (end of interrupt) register
#define RRD     0xC0  // remote read register
#define IDR     0xD0  // logical destination register
#define DFR     0xE0  // destination format register
#define SVR     0xF0  // spurious interrupt vector register 
#define ESR     0x280  // error status register

/* interrupt command register has two parts */
#define ICRLO   0x300  // interrupt command 0
#define ICRL1   0x310  // interrupt command 1
                        
/* LVT registers */
#define TIMER   0x320  // timer register
#define PCR     0x340  // performance counter
#define LINT0   0x350  // local vector table 1
#define LINT1   0x360  // local vector table 2
#define ERROR   0x370  // local vector table 3
#define TIC     0x380  // timer initial counter
#define TCC     0x390  // timer current counter 
#define TDV     0x3E0  // timer divide configure



volatile uint32_t *lapic;



bool apic_check() {

}
