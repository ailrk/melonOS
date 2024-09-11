#include "trap.h"
#include "err.h"
#include "kbd.h"
#include "pic.h"
#include "tty.h"
#include "idt.h"

#define DEBUG 1

extern void * vectors[];
unsigned int ticks;


#if DEBUG
static void dump_trapframe(const TrapFrame *tf) {
    tty_printf("trapframe> \n");
    tty_printf(" edi:    %x\n", tf->edi);
    tty_printf(" esi:    %x\n", tf->esi);
    tty_printf(" ebp:    %x\n", tf->ebp);
    tty_printf(" ebx:    %x\n", tf->ebx);
    tty_printf(" edx:    %x\n", tf->edx);
    tty_printf(" ecx:    %x\n", tf->ecx);
    tty_printf(" eax:    %x\n", tf->eax);
    tty_printf(" gs:     %x\n", tf->gs);
    tty_printf(" fs:     %x\n", tf->fs);
    tty_printf(" es:     %x\n", tf->es);
    tty_printf(" ds:     %x\n", tf->ds);
    tty_printf(" trapno: %x\n", tf->trapno);
    tty_printf(" err:    %x\n", tf->err);
    tty_printf(" eip:    %x\n", tf->eip);
    tty_printf(" cs:     %x\n", tf->cs);
    tty_printf(" elfags: %x\n", tf->eflags);
    tty_printf(" esp:    %x\n", tf->esp);
    tty_printf(" ss:     %x\n", tf->ss);
}
#endif


void trap_init() {
    for (int i = 0; i < 256; ++i) {
        regist_idt_handler(i, vectors[i], InterruptGate);
    }
    regist_idt_handler(I_SYSCALL, vectors[I_SYSCALL], TrapGate);
}


void handle_I_IRQ_TIMER() {
    ticks++;
    pic_eoi();
}


void handle_I_IRQ_KBD() {
    char c;
    kbd_handler();
    if ((c = kbd_getc()) != -1) {
        tty_printf("%c", c);
    }
    pic_eoi();
}


void handle_I_IRQ_COM2() {
    tty_write_string("irq com2\n");
    pic_eoi();
}

void handle_I_IRQ_COM1() {
    tty_write_string("irq com1\n");
    pic_eoi();
}

void handle_I_IRQ_LPT1() {
    tty_write_string("irq lpt1\n");
    pic_eoi();
}

void handle_I_IRQ_CMOS() {
    tty_write_string("irq cmos timer\n");
    pic_eoi();
}

void handle_I_IRQ_MOUSE() {
    tty_write_string("irq mouse\n");
    pic_eoi();
}

void handle_I_IRQ_IDE() {
    tty_write_string("irq ide\n");
    pic_eoi();
}

void handle_I_IRQ_ERR() {
    tty_write_string("irq err\n");
    pic_eoi();
}

void handle_I_IRQ_SPURIOUS(const TrapFrame *tf) {
    tty_printf("[cpu]: spurious interrupt at %x:%x\n", tf->cs, tf->eip);
    pic_eoi();
}


/* trap handler */
void trap(TrapFrame *tf) {
    switch (tf->trapno) {
        case MAP_IRQ(I_IRQ_TIMER):
            handle_I_IRQ_TIMER();
            break;
        case MAP_IRQ(I_IRQ_KBD): 
            handle_I_IRQ_KBD();
            break;
        case MAP_IRQ(I_IRQ_COM2):
            handle_I_IRQ_CMOS();
            break;
        case MAP_IRQ(I_IRQ_COM1):
            handle_I_IRQ_COM1();
            break;
        case MAP_IRQ(I_IRQ_LPT1):
            handle_I_IRQ_LPT1();
            break;
        case MAP_IRQ(I_IRQ_CMOS):
            handle_I_IRQ_CMOS();
            break;
        case MAP_IRQ(I_IRQ_MOUSE):
            handle_I_IRQ_MOUSE();
            break;
        case MAP_IRQ(I_IRQ_IDE):
            handle_I_IRQ_IDE();
            break;
        case MAP_IRQ(I_IRQ_ERR):
            handle_I_IRQ_ERR();
            break;
        case MAP_IRQ(I_IRQ_SPURIOUS):
            handle_I_IRQ_SPURIOUS(tf);
            break;
        case I_SYSCALL:
            break;
        default:
            panic("trap");
    }
}
