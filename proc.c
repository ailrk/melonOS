#include "proc.h"
#include "debug.h"
#include "defs.h"
#include "err.h"
#include "i386.h"
#include "mem.h"
#include "ncli.h"
#include "palloc.h"
#include "trap.h"
#include "tty.h"
#include "string.h"
#include "vmem.h"
#include <stdint.h>

#define DEBUG 1

extern void trapret();
extern void swtch(Context **save, Context *load);
CPU cpu;


typedef struct PTable {
    Process t[NPROC];
} PTable;

PTable ptable;

int nextpid = 1;


#if DEBUG
void dump_context(const Context *c) {
    if (!c) {
        debug_printf("[CTX| no context yet]");
        return;
    }
    debug_printf("[CTX|%#x, %#x, %#x, %#x, %#x]",
                 c->edi, c->esi, c->ebx, c->ebp, c->eip);
}

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

    debug_printf("[PROC|%d,%s,%s,", p->pid, p->name, state);
    dump_context(p->context);
    debug_printf("]");
    dump_context(p->context);

}
#endif


/* Return the running cpu */
CPU *this_cpu() {
    if (readeflags() & FL_IF)
        panic("interrupt is enabled on `this_cpu` call");

    return &cpu;
}

/* Return the current process */
Process *this_proc() {
    Process *p;
    push_cli();
    p = this_cpu()->proc;
    pop_cli();
    return p;
}

/*! Return a forked child process to the user space */
void forkret() {
}


/*! Try to get the next unused process in ptable */
static Process *get_unused_process() {
    Process *p;
    bool found = false;
    for (int i = 1; i < NPROC; ++i) {
        p = &ptable.t[i];
        if (p->state == PROC_UNUSED) {
            found = true;
            break;
        }
    }
    return p;
}


/*! The process's kernel stack has the following setup:
 *
 * sp   +---------------+
 *      |  |eip=0       |
 *      |  |...         |
 *      | TrapFrame     |
 *      +---------------+
 *      | trapret       |
 *      +---------------+
 *      |  |eip=forkret | 
 *      |  |...         |
 *      | Context       |
 *      +---------------+
 *      |  ...          |
 * kstk +---------------+
 *
 *  When scheduler is trying to switch in to the process, swtch will pop
 *  all context except the eip, when it `ret`, the control will be transferred 
 *  to `eip` in this case we set to `forkret.`
 *  `forkret` simply return to caller. In this case it will return to `trapret`,
 *  which will further pop all registers in the trap frame. We set the eip in
 *  the trapframe to 0, so after `trapret` the process will start running on
 *  the address 0x0.
 */
static bool setup_process_stack(Process *p) {
    // allocate and build the kernel stack
    if ((p->kstack = palloc()) == 0) {
        tty_printf("found %#x", p);
        p->state = PROC_UNUSED;
        return false;
    }

    char *sp = p->kstack + KSTACK_SZ;

    sp -= sizeof(TrapFrame);
    p->trapframe = (TrapFrame*)sp;

    sp -= sizeof(uintptr_t);
    *(uintptr_t*)sp = (uintptr_t)trapret;

    sp -= sizeof(Context);
    p->context = (Context*)sp;
    memset(p->context, 0, sizeof(Context));

    // swtch returns on `forkret`
    p->context->eip = (uint32_t)forkret;
    return true;
}


/*! Allocate a new process and set it up to run in kernel. */
static Process *allocate_process() {
    Process *p = get_unused_process();

    p->state = PROC_CREATED;
    p->pid = nextpid++;

    if (!setup_process_stack(p)) return 0;
    return p;
}


/*! Setup the trapframe for the first process to create an illusion 
 * that a trap occured. So if we call trapret, it will pop all registers
 * in the trap frame hence switch the control.
 * */
static void set_pid1_trapframe(Process *p) {
    memset(p->trapframe, 0, sizeof(*p->trapframe));
    p->trapframe->cs = (SEG_UCODE << 3) | DPL_U; // set segment DPL
    p->trapframe->ds = (SEG_UDATA << 3) | DPL_U; 
    p->trapframe->es = p->trapframe->ds;
    p->trapframe->ss = p->trapframe->ds;
    p->trapframe->eflags = FL_IF;
    p->trapframe->esp = PAGE_SZ;
    p->trapframe->eip = 0; // start of the program
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
    set_pid1_trapframe(p);
    strncpy(p->name, "init", sizeof(p->name));
    p->state = PROC_READY;
    tty_printf("\033[32mok\033[0m\n");
}


/*! Grow process user memory by n bytes, n can be negative.
 * */
bool grow_process(int n) {
    if (n == 0)
        return true;

    Process *p = this_proc();
    size_t sz = p->size;

    if (n > 0) {
        if ((sz = allocate_user_vmem(p->page_table, sz, sz + n)) == 0) {
            return false;
        }
    } else {
        if ((sz = allocate_user_vmem(p->page_table, sz, sz + n)) == 0) 
            return false;
    }

    p->size = sz;
    switch_user_vmem(p);
    return true;
}


/*! The scheduler next returns. CPU loops over the `ptable` and look
 *  for the next process to run. The loop will pick the first ready 
 *  process, switch to run the process, wait until the process switch 
 *  back to the scheduler.  
 * */
void scheduler() {
    tty_printf("[\033[32mboot\033[0m] scheduler...\n");
    CPU *cpu = this_cpu();
    cpu->proc = 0;
    sti();

    for(;;) {
        for (Process *p = ptable.t; p < ptable.t + NPROC; ++p) {
            if (p->state != PROC_READY)
                continue;
            cpu->proc = p;

            switch_user_vmem(p);
            p->state = PROC_RUNNING;
            swtch(&cpu->scheduler, p->context);
            switch_kernel_vmem();
            cpu->proc = 0;
        }
    }
}


void sched() {
    Process *p = this_proc();

    if(this_cpu()->ncli != 1)
        panic("sched locks");

    if(p->state == PROC_RUNNING)
        panic("sched running");

    if(readeflags() & FL_IF)
        panic("sched interruptible");
     
    int int_on = this_cpu()->int_on;
    swtch(&p->context, this_cpu()->scheduler);
    this_cpu()->int_on = int_on;
}
