#pragma once

#include <stdint.h>

void handle_I_DIVBYZERO();
void handle_I_DEBUG();
void handle_I_NMI();
void handle_I_BRKPNT();
void handle_I_OVERFLOW();
void handle_I_BOUND();
void handle_I_ILLEGALOP();
void handle_I_COPNOAVIL();
void handle_I_DOUBLEFLT();
void handle_I_COPSEG();
void handle_I_TSS();
void handle_I_SEGNP();
void handle_I_STKSGFLT();
void handle_I_GPFLT();
void handle_I_PGFLT();
void handle_I_FPERR();
void handle_I_ALIGN();
void handle_I_MACHINE();
void handle_I_SIMDERR();
void handle_I_IRQ_TIMER();
void handle_I_IRQ_KBD();
void handle_I_IRQ_COM1();
void handle_I_IRQ_IDE();
void handle_I_IRQ_ERR();
void handle_I_IRQ_SPURIOUS();
void handle_I_SYSCALL();
void handle_I_DEFAULT();

void isr_register();


typedef struct TrapFrame {
    uint32_t esi;
}TrapFrame;
