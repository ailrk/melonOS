#include "proc.h"
#include "defs.h"
#include "err.h"
#include "i386.h"
#include "mem.h"
#include "palloc.h"
#include "trap.h"
#include "tty.h"
#include "string.h"
#include "vmem.h"
#include <stdint.h>

#define DEBUG 1

extern void trapret();
CPU cpu;


Process ptable[NPROC + 1];
int nextpid = 1;


#if DEBUG
void dump_process(const Process *p) {
    if (p->pid < 0 || p->pid > NPROC)
        return;
    
    char *state;
    switch(p->state) {
        case PROC_UNUSED:
            state = "unused";
            break;
        case PROC_CREATED:
            state = "created";
            break;
        case PROC_SLEEPING:
            state = "sleeping";
            break;
        case PROC_READY:
            state = "ready";
            break;
        case PROC_RUNNING:
            state = "running";
            break;
        case PROC_ZOMBIE:
            state = "zombie";
            break;
        default:
            state = "?";
            break;
    }

    tty_printf("[%d,%s,%s]", p->pid, p->name, state);
}


void dump_processes() {
    for (int i = 0; i < NPROC; ++i) {
        Process p = ptable[i];
    }
}
#endif

/*! Return a forked child process to the user space */
void forkret() {
}


/*! Allocate a new process and set it up to run in kernel. */
static Process *allocate_process() {
    Process *p;
    bool found = false;

    for (int i = 1; i < NPROC; ++i) {
        p = &ptable[i];
        if (p->state == PROC_UNUSED) {
            found = true;
            break;
        }
    }

    p->state = PROC_CREATED;
    p->pid = nextpid++;

    // allocate and build the kernel stack
    if ((p->kstack = palloc()) == 0) {
        tty_printf("found %x", p);
        p->state = PROC_UNUSED;
        return 0;
    }

    char *sp = p->kstack + KSTACK_SZ;

    // reserving space for trapframe and context
    sp -= sizeof(TrapFrame);
    p->trapframe = (TrapFrame*)sp;

    sp -= sizeof(uint32_t);
    *sp = (uint32_t)trapret;

    sp -= sizeof(Context);
    p->context = (Context*)sp;
    memset(p->context, 0, sizeof(Context));
    p->context->eip = (uint32_t)forkret;

    return p;
}


/*! Initialize the first user space process */
void init_pid1() {
    tty_printf("[\033[32mboot\033[0m] init1...");
    Process *p;
    if ((p = allocate_process()) == 0) {
        panic("init_pid, failed to allocate process");
    }

    if ((p->page_table = setup_kernel_vmem()) == 0)
        panic("init_pid1");

    extern char __INIT1_BEGIN__[];
    extern char __INIT1_END__[];
    int init1_sz = __INIT1_END__ - __INIT1_BEGIN__;

    init_user_vmem(p->page_table, __INIT1_BEGIN__, init1_sz); 
    p->size = PAGE_SZ;
    memset(p->trapframe, 0, sizeof(*p->trapframe));
    
    p->trapframe->cs = (SEG_UCODE << 3) | DPL_U; // set segment DPL
    p->trapframe->ds = (SEG_UDATA << 3) | DPL_U; 
    p->trapframe->es = p->trapframe->ds;
    p->trapframe->ss = p->trapframe->ds;
    p->trapframe->eflags = FL_IF;
    p->trapframe->esp = PAGE_SZ;
    p->trapframe->eip = 0;

    strncpy(p->name, "init", sizeof(p->name));
    p->state = PROC_READY;
    tty_printf("\033[32mok\033[0m\n");
}
