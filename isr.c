#include "isr.h"
#include "idt.h"
#include "tty.h"
#include "kbd.h"
#include "pic.h"
#include <stdint.h>


void handle_I_DIVBYZERO() {
    tty_write_string("divided by zero \n");
}

void handle_I_DEBUG() {
    tty_write_string("debug \n");
}

void handle_I_NMI() {
    tty_write_string("NMI \n");
}

void handle_I_BRKPNT() {
    tty_write_string("breakpoint\n");
}

void handle_I_OVERFLOW() {
    tty_write_string("overflow\n");
}

void handle_I_BOUND() {
    tty_write_string("bound\n");
}

void handle_I_ILLEGALOP() {
    tty_write_string("illegal op\n");
}

void handle_I_COPNOAVIL() {
    tty_write_string("coprocessor not available\n");
}

void handle_I_DOUBLEFLT() {
    tty_write_string("double fault \n");
}

void handle_I_COPSEG() {
    tty_write_string("coprocessor sement overrun\n");
}

void handle_I_TSS() {
    tty_write_string("invalid tss\n");
}

void handle_I_SEGNP() {
    tty_write_string("segment not present\n");
}

void handle_I_STKSGFLT() {
    tty_write_string("stack segment fault\n");
}

void handle_I_GPFLT() {
    tty_write_string("general protection fault\n");
}

void handle_I_PGFLT() {
    tty_write_string("page fault\n");
}

void handle_I_FPERR() {
    tty_write_string("floating point error\n");
}

void handle_I_ALIGN() {
    tty_write_string("alignment check\n");
}

void handle_I_MACHINE() {
    tty_write_string("machine check\n");
}

void handle_I_SIMDERR() {
    tty_write_string("simd error\n");
}

/* hardware interrupts */
void handle_I_IRQ_TIMER() {
    pic_send_eoi(I_IRQ_TIMER);
}

void handle_I_IRQ_KBD() {
    unsigned char c = kbd_getc();
    tty_putchar(c);
    pic_send_eoi(I_IRQ_KBD);
}

void handle_I_IRQ_COM2() {
    tty_write_string("irq com2\n");
    pic_send_eoi(I_IRQ_COM2);
}

void handle_I_IRQ_COM1() {
    tty_write_string("irq com1\n");
    pic_send_eoi(I_IRQ_COM1);
}

void handle_I_IRQ_LPT1() {
    tty_write_string("irq lpt1\n");
    pic_send_eoi(I_IRQ_LPT1);
}

void handle_I_IRQ_CMOS() {
    tty_write_string("irq cmos timer\n");
    pic_send_eoi(I_IRQ_CMOS);
}

void handle_I_IRQ_MOUSE() {
    tty_write_string("irq mouse\n");
    pic_send_eoi(I_IRQ_MOUSE);
}


void handle_I_IRQ_IDE() {
    tty_write_string("irq ide\n");
    pic_send_eoi(I_IRQ_IDE);
}

void handle_I_IRQ_ERR() {
    tty_write_string("irq err\n");
    pic_send_eoi(I_IRQ_ERR);
}

void handle_I_IRQ_SPURIOUS() {
    tty_write_string("irq spurious\n");
    pic_send_eoi(I_IRQ_SPURIOUS);
}

/* custom interrupt handlers */
void handle_I_SYSCALL() {
    tty_write_string("syscall\n");
}

void handle_I_DEFAULT() {
}

// default exception handler 
__attribute__((noreturn))
void exception_handler() {
    tty_printf("\n[ERR] unhandled exception\n");
    __asm__ volatile ("cli; hlt"); // hangs the computer
}


extern void* isr_I_DIVBYZERO;
extern void* isr_I_DEBUG;
extern void* isr_I_NMI;
extern void* isr_I_BRKPNT;
extern void* isr_I_OVERFLOW;
extern void* isr_I_BOUND;
extern void* isr_I_ILLEGALOP;
extern void* isr_I_COPNOAVIL;
extern void* isr_I_DOUBLEFLT;
extern void* isr_I_COPSEG;
extern void* isr_I_TSS;
extern void* isr_I_SEGNP;
extern void* isr_I_STKSGFLT;
extern void* isr_I_GPFLT;
extern void* isr_I_PGFLT;
extern void* isr_I_FPERR;
extern void* isr_I_ALIGN;
extern void* isr_I_MACHINE;
extern void* isr_I_SIMDERR;
extern void* isr_I_IRQ_TIMER;
extern void* isr_I_IRQ_KBD;
extern void* isr_I_IRQ_COM2;
extern void* isr_I_IRQ_COM1;
extern void* isr_I_IRQ_LPT1;
extern void* isr_I_IRQ_CMOS;
extern void* isr_I_IRQ_MOUSE;
extern void* isr_I_IRQ_IDE;
extern void* isr_I_IRQ_ERR;
extern void* isr_I_IRQ_SPURIOUS;
extern void* isr_I_SYSCALL;
extern void* isr_I_DEFAULT;



void isr_register() {
    for (int i = 0; i < IDT_MAX_VECTOR; ++i) {
        regist_idt_handler(i, &exception_handler, InterruptGate);
    }
    regist_idt_handler(I_DIVBYZERO, &isr_I_DIVBYZERO, InterruptGate);
    regist_idt_handler(I_DEBUG, &isr_I_DEBUG, InterruptGate);
    regist_idt_handler(I_NMI, &isr_I_NMI, InterruptGate);
    regist_idt_handler(I_BRKPNT, &isr_I_BRKPNT, InterruptGate);
    regist_idt_handler(I_OVERFLOW, &isr_I_OVERFLOW, InterruptGate);
    regist_idt_handler(I_BOUND, &isr_I_BOUND, InterruptGate);
    regist_idt_handler(I_ILLEGALOP, &isr_I_ILLEGALOP, InterruptGate);
    regist_idt_handler(I_COPNOAVIL, &isr_I_COPNOAVIL, InterruptGate);
    regist_idt_handler(I_DOUBLEFLT, &isr_I_DOUBLEFLT, InterruptGate);
    regist_idt_handler(I_COPSEG, &isr_I_COPSEG, InterruptGate);
    regist_idt_handler(I_TSS, &isr_I_TSS, InterruptGate);
    regist_idt_handler(I_SEGNP, &isr_I_SEGNP, InterruptGate);
    regist_idt_handler(I_STKSGFLT, &isr_I_STKSGFLT, InterruptGate);
    regist_idt_handler(I_GPFLT, &isr_I_GPFLT, InterruptGate);
    regist_idt_handler(I_PGFLT, &isr_I_PGFLT, InterruptGate);
    regist_idt_handler(I_FPERR, &isr_I_FPERR, InterruptGate);
    regist_idt_handler(I_ALIGN, &isr_I_ALIGN, InterruptGate);
    regist_idt_handler(I_MACHINE, &isr_I_MACHINE, InterruptGate);
    regist_idt_handler(I_SIMDERR, &isr_I_SIMDERR, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_TIMER), &isr_I_IRQ_TIMER, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_KBD), &isr_I_IRQ_KBD, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_COM2), &isr_I_IRQ_COM2, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_COM1), &isr_I_IRQ_COM1, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_LPT1), &isr_I_IRQ_LPT1, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_CMOS), &isr_I_IRQ_CMOS, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_MOUSE), &isr_I_IRQ_MOUSE, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_IDE), &isr_I_IRQ_IDE, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_ERR), &isr_I_IRQ_ERR, InterruptGate);
    regist_idt_handler(MAP_IRQ(I_IRQ_SPURIOUS), &isr_I_IRQ_SPURIOUS, InterruptGate);

    regist_idt_handler(I_SYSCALL, &isr_I_SYSCALL, TrapGate);
    regist_idt_handler(I_DEFAULT, &isr_I_DEFAULT, TrapGate);
}
