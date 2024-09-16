#include "trap.h"
#include "debug.h"
#include "err.h"
#include "kbd.h"
#include "pic.h"
#include "proc.h"
#include "traps.h"
#include "syscall.h"
#include "tty.h"
#include "idt.h"

#define DEBUG 1

extern void * vectors[];
unsigned int ticks;


#if DEBUG
static void dump_trapframe(const TrapFrame *tf) {
    debug_printf("trapframe> \n");
    debug_printf(" edi:    %#x\n", tf->edi);
    debug_printf(" esi:    %#x\n", tf->esi);
    debug_printf(" ebp:    %#x\n", tf->ebp);
    debug_printf(" ebx:    %#x\n", tf->ebx);
    debug_printf(" edx:    %#x\n", tf->edx);
    debug_printf(" ecx:    %#x\n", tf->ecx);
    debug_printf(" eax:    %#x\n", tf->eax);
    debug_printf(" gs:     %#x\n", tf->gs);
    debug_printf(" fs:     %#x\n", tf->fs);
    debug_printf(" es:     %#x\n", tf->es);
    debug_printf(" ds:     %#x\n", tf->ds);
    debug_printf(" trapno: %#x\n", tf->trapno);
    debug_printf(" err:    %#x\n", tf->err);
    debug_printf(" eip:    %#x\n", tf->eip);
    debug_printf(" cs:     %#x\n", tf->cs);
    debug_printf(" elfags: %#x\n", tf->eflags);
    debug_printf(" esp:    %#x\n", tf->esp);
    debug_printf(" ss:     %#x\n", tf->ss);
}
#endif


void trap_init() {
    for (int i = 0; i < 256; ++i) {
        regist_idt_handler(i, vectors[i], InterruptGate);
    }
    regist_idt_handler(I_SYSCALL, vectors[I_SYSCALL], TrapGate);
}

/*! When a system call is invoked, the system call number is
 *  moved to eax and `int I_SYSCALL` is performed, which
 *  causes the trap to dispatch to this handler.
 *
 *  `handle_syscall` will set the trapframe to the current
 *  process, then it dispatches to `syscall`, the
 *
 *  After trap is invoked, `trapret` will bring the program back
 *  to the user space with `iret`.
 * */
void handle_syscall(TrapFrame *tf) {
    if (this_proc()->killed)
        exit();
    this_proc()->trapframe = tf;
    syscall();
    if (this_proc()->killed)
        exit();
    return;
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
        debug_printf("%c", c);
    }
    pic_eoi();
}


void handle_I_IRQ_COM2() {
    debug_printf("irq com2\n");
    pic_eoi();
}

void handle_I_IRQ_COM1() {
    debug_printf("irq com1\n");
    pic_eoi();
}

void handle_I_IRQ_LPT1() {
    debug_printf("irq lpt1\n");
    pic_eoi();
}

void handle_I_IRQ_CMOS() {
    debug_printf("irq cmos timer\n");
    pic_eoi();
}

void handle_I_IRQ_MOUSE() {
    debug_printf("irq mouse\n");
    pic_eoi();
}

void handle_I_IRQ_IDE() {
    debug_printf("irq ide\n");
    pic_eoi();
}

void handle_I_IRQ_ERR() {
    debug_printf("irq err\n");
    pic_eoi();
}

void handle_I_IRQ_SPURIOUS(const TrapFrame *tf) {
    debug_printf("[cpu]: spurious interrupt at %#x:%#x\n", tf->cs, tf->eip);
    pic_eoi();
}

/* trap handler */
void trap(TrapFrame *tf) {
    switch (tf->trapno) {
        case I_SYSCALL:
        case MAP_IRQ(I_IRQ_TIMER):
            handle_I_IRQ_TIMER();
            break;
        case MAP_IRQ(I_IRQ_KBD):
            handle_I_IRQ_KBD();
            break;
        case MAP_IRQ(I_IRQ_COM2):
            handle_I_IRQ_COM2();
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
        default:
#if DEBUG
            dump_trapframe(tf);
            panic("trap");
#endif
    }
}
