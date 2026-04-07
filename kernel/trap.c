#include "trap.h"
#include "console.h"
#include "debug.h"
#include "err.h"
#include "mmu.h"
#include "pdefs.h"
#include "print.h"
#include "process.h"
#include "ps2.h"
#include "sys/syscall.h"
#include "driver/kbd.h"
#include "driver/pic.h"
#include "fs/disk.h"
#include "trap/idt.h"
#include "trap/traps.h"
#include "uart.h"


extern void *vectors[];
unsigned     ticks;


static char *traperrs[] = {
    [I_DIVBYZERO]             = "I_DIVBYZERO",
    [I_DEBUG]                 = "I_DEBUG",
    [I_NMI]                   = "I_NMI",
    [I_BRKPNT]                = "I_BRKPNT",
    [I_OVERFLOW]              = "I_OVERFLOW",
    [I_BOUND]                 = "I_BOUND",
    [I_ILLEGALOP]             = "I_ILLEGALOP",
    [I_COPNOAVIL]             = "I_COPNOAVIL",
    [I_DOUBLEFLT]             = "I_DOUBLEFLT",
    [I_COPSEG]                = "I_COPSEG",
    [I_TSS]                   = "I_TSS",
    [I_SEGNP]                 = "I_SEGNP",
    [I_STKSGFLT]              = "I_STKSGFLT",
    [I_GPFLT]                 = "I_GPFLT",
    [I_PGFLT]                 = "I_PGFLT",
    [0xf]                     = "0xf",
    [I_FPERR]                 = "I_FPERR",
    [I_ALIGN]                 = "I_ALIGN",
    [I_MACHINE]               = "I_MACHINE",
    [I_SIMDERR]               = "I_SIMDERR",
    [MAP_IRQ(I_IRQ_TIMER)]    = "I_IRQ_TIMER",
    [MAP_IRQ(I_IRQ_KBD)]      = "I_IRQ_KBD",
    [MAP_IRQ(I_IRQ_CASCADE)]  = "I_IRQ_CASCADE",
    [MAP_IRQ(I_IRQ_COM2)]     = "I_IRQ_COM2",
    [MAP_IRQ(I_IRQ_COM1)]     = "I_IRQ_COM1",
    [MAP_IRQ(I_IRQ_LPT1)]     = "I_IRQ_LPT1",
    [MAP_IRQ(I_IRQ_CMOS)]     = "I_IRQ_CMOS",
    [MAP_IRQ(I_IRQ_MOUSE)]    = "I_IRQ_MOUSE",
    [MAP_IRQ(I_IRQ_IDE)]      = "I_IRQ_IDE",
    [MAP_IRQ(I_IRQ_ERR)]      = "I_IRQ_ERR",
    [MAP_IRQ(I_IRQ_SPURIOUS)] = "I_IRQ_SPURIOUS",
    [I_SYSCALL]               = "I_SYSCALL",
};


#if DEBUG
void dump_trapframe(const TrapFrame *tf) {
    debug("trapframe> \n");
    debug(" edi:    %#x\n", tf->edi);
    debug(" esi:    %#x\n", tf->esi);
    debug(" ebp:    %#x\n", tf->ebp);
    debug(" ebx:    %#x\n", tf->ebx);
    debug(" edx:    %#x\n", tf->edx);
    debug(" ecx:    %#x\n", tf->ecx);
    debug(" eax:    %#x\n", tf->eax);
    debug(" gs:     %#x\n", tf->gs);
    debug(" fs:     %#x\n", tf->fs);
    debug(" es:     %#x\n", tf->es);
    debug(" ds:     %#x\n", tf->ds);
    debug(" trapno: %#x, %s\n", tf->trapno, traperrs[tf->trapno]);
    debug(" err:    %#x\n", tf->err);
    debug(" eip:    [<%#x>]\n", tf->eip);
    debug(" cs:     %#x\n", tf->cs);
    debug(" eflags: %#x\n", tf->eflags);
    debug(" esp:    %#x\n", tf->esp);
    debug(" ss:     %#x\n", tf->ss);
}
#endif


/* Input source */
uint8_t source_uart() { return uart_getc(COM1); }


/* Initialized the trap. */
void trap_init() {
    dprintf(LOG_BOOT " trap_init...\n");
    for (int i = 0; i < 256; ++i) {
        regist_idt_handler(
                i,
                vectors[i],
                GATE_P(1) | GATE_DPL(DPL_K) | INT_GATE);
    }
    regist_idt_handler(
            I_SYSCALL,
            vectors[I_SYSCALL],
            GATE_P(1) | GATE_DPL(DPL_U) | TRAP_GATE);

    idt_init();
    pic_init();
    dprintf(LOG_BOOT " trap_init " LOG_OK "\n");
}


/* When a system call is invoked, the system call number is
 * moved to eax and `int I_SYSCALL` is performed, which
 * causes the trap to dispatch to this handler.
 *
 * `handle_syscall` will set the trapframe to the current
 * process, then it dispatches to `syscall`, the
 *
 * After trap is invoked, `trapret` will bring the program back
 * to the user space with `iret`.
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


void handle_I_IRQ_TIMER(const TrapFrame *tf) {
    ticks++;
    pic_eoi();
}


void handle_I_IRQ_KBD() {
    debug("irq ps2\n");
    kbd_read();
    console_handler();
    pic_eoi();
}


void handle_I_IRQ_COM2() {
    #ifdef DEBUG
    debug("irq com2\n");
    #endif
    pic_eoi();
}


void handle_I_IRQ_COM1() {
    debug("irq com1\n");
    uart_read(COM1);
    console_handler();
    pic_eoi();
}


void handle_I_IRQ_LPT1() {
    #ifdef DEBUG
    debug("irq lpt1\n");
    #endif
    pic_eoi ();
}


void handle_I_IRQ_CMOS() {
    #ifdef DEBUG
    debug("irq cmos timer\n");
    #endif
    pic_eoi();
}


void handle_I_IRQ_MOUSE() {
    #ifdef DEBUG
    debug("irq mouse\n");
    #endif
    pic_eoi();
}


void handle_I_IRQ_IDE() {
    disk_handler();
    pic_eoi();
}


void handle_I_IRQ_ERR() {
    #ifdef DEBUG
    debug("irq err\n");
    #endif
    pic_eoi();
}


void handle_I_IRQ_SPURIOUS(const TrapFrame *tf) {
    #ifdef DEBUG
    debug("cpu: spurious interrupt at %#x:%#x\n", tf->cs, tf->eip);
    #endif
    pic_eoi();
}


/* trap handler */
void trap(TrapFrame *tf) {
    switch (tf->trapno) {
        case I_SYSCALL:
            handle_syscall(tf);
            break;
        case MAP_IRQ(I_IRQ_TIMER):
            handle_I_IRQ_TIMER(tf);
            break;
        case MAP_IRQ (I_IRQ_KBD):
            handle_I_IRQ_KBD();
            break;
        case MAP_IRQ(I_IRQ_COM2):
            handle_I_IRQ_COM2();
            break;
        case MAP_IRQ (I_IRQ_COM1):
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
#endif
            // kernel should never ends up here
            if (!this_proc() || (tf->cs & 3) == DPL_K) {
                panic("trap");
            }

            // it's from an errorous user space program.
            perror("trap");
            this_proc()->killed = 1;
    }

    // Exit killed user space process
    if (this_proc() && this_proc()->killed && (tf->cs & 3) == DPL_U) {
        exit();
    }

    // Yield running user process. This brings the CPU back
    // to the scheduler.
    // See trapasm.s to see the stack setup at this point.
    if (this_proc() && this_proc()->state == PROC_RUNNING && (tf->cs & 3) == DPL_U) {
#if DEBUG
            debug("YEILDING\n");
            dump_trapframe(tf);
#endif
        yield();
    }
}
