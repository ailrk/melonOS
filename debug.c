#include "tty.h"
#include "idt.h"
#include "i386.h"


void print_all_interrupts() {
    vga_tty_init();
    vga_tty_write_string(">> \n");
    int_(I_DIVBYZERO);
    int_(I_DEBUG);
    int_(I_NMI);
    int_(I_BRKPNT);
    int_(I_OVERFLOW);
    int_(I_BOUND);
    int_(I_ILLEGALOP);
    int_(I_COPNOAVIL);
    int_(I_DOUBLEFLT);
    int_(I_COPSEG);
    int_(I_TSS);
    int_(I_SEGNP);
    int_(I_STKSGFLT);
    int_(I_GPFLT);
    int_(I_PGFLT);
    int_(I_FPERR);
    int_(I_ALIGN);
    int_(I_MACHINE);
    int_(I_SIMDERR);
    int_(I_IRQ_TIMER);
    int_(I_IRQ_KBD);
    int_(I_IRQ_COM1);
    int_(I_IRQ_IDE);
    int_(I_IRQ_ERR);
    int_(I_IRQ_SPURIOUS);
    int_(I_SYSCALL);
    int_(I_DEFAULT);
    vga_tty_write_string("<< \n");
}
