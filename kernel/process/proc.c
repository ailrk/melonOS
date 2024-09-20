#include <stdint.h>
#include "debug.h"
#include "defs.h"
#include "err.h"
#include "i386.h"
#include "mem.h"
#include "ncli.h"
#include "pdefs.h"
#include "trap.h"
#include "tty.h"
#include "string.h"
#include "spinlock.h"
#include "process/proc.h"
#include "mem/palloc.h"
#include "mem/vmem.h"

#define DEBUG 1

extern void trapret();
extern void swtch(Context **save, Context *load);
CPU cpu;


typedef struct PTable {
    SpinLock lk;
    Process t[NPROC];
} PTable;

PTable ptable;

int nextpid = 1;
Process *proc_init1;

void ptable_init() {
    ptable.lk = new_lock("ptable.lk");
}


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
    if (p->pid > NPROC)
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
    for (int i = 1; i < NPROC; ++i) {
        p = &ptable.t[i];
        if (p->state == PROC_UNUSED) {
            return p;
        }
    }
    return 0;
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
    if (nextpid > NPROC)
        panic("allocate_process: can't allocate more pids");
    lock(&ptable.lk);
    Process *p = get_unused_process();
    if (!p) {
        unlock(&ptable.lk);
        return 0;
    }
    p->state = PROC_CREATED;
    p->pid = nextpid++;
    unlock(&ptable.lk);
    if (!setup_process_stack(p)) return 0;
    return p;
}


/*! Deallocate a process.
 *  This is the all purpose deallocation, it tries to free all resources hold by
 *  the process.
 * */
static void deallocate_process(Process *p) {
    lock(&ptable.lk);
    p->size = 0;
    if (p->pgdir) {
        free_vmem(p->pgdir);
        p->pgdir = 0;
    }
    if (p->kstack)  {
        pfree(p->kstack);
        p->kstack = 0;
    }
    p->kstack = 0;
    p->pid = 0;
    p->parent = 0;
    p->state = PROC_UNUSED;
    p->trapframe = 0;
    p->context = 0;
    p->chan = 0;
    p->killed = 0;
    memset(p->name, 0, sizeof(p->name));
    unlock(&ptable.lk);
}


/*! Setup the trapframe for the first process to create an illusion
 *  that a trap occured. So if we call trapret, it will pop all registers
 *  in the trap frame hence switch the control.
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


/*! Initialize the first user space process. */
void init_pid1() {
    tty_printf("[\033[32mboot\033[0m] init1...");
    Process *p;
    if ((p = allocate_process()) == 0) {
        panic("init_pid1: failed to allocate process");
    }

    if ((p->pgdir = allocate_kernel_vmem()) == 0)
        panic("init_pid1");

    extern char __INIT1_BEGIN__[];
    extern char __INIT1_END__[];
    int init1_sz = __INIT1_END__ - __INIT1_BEGIN__;

    init_user_vmem(p->pgdir, __INIT1_BEGIN__, init1_sz);
    p->size = PAGE_SZ;
    set_pid1_trapframe(p);
    strncpy(p->name, "init", sizeof(p->name));

    lock(&ptable.lk);
    p->state = PROC_READY;
    proc_init1 = p;
    unlock(&ptable.lk);

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
        if ((sz = allocate_user_vmem(p->pgdir, sz, sz + n)) == 0) {
            return false;
        }
    } else {
        if ((sz = allocate_user_vmem(p->pgdir, sz, sz + n)) == 0)
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
    for(;;) {
        sti(); // force enable interrupt
        lock(&ptable.lk);
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
        unlock(&ptable.lk);
    }
}


/*! Fork a process
 *  @return  return 0 on child process, pid of the child process on parent. -1
 *           if failed.
 * */
int fork() {
    Process *child;
    Process *thisp = this_proc();

    if ((child = allocate_process()) == 0) {
        return -1;
    }

    if ((child->pgdir = copy_user_vmem(thisp->pgdir, thisp->size)) == 0) {
        deallocate_process(child);
        return -1;
    }

    child->size = thisp->size;
    child->parent = thisp;
    *child->trapframe = *thisp->trapframe;
    child->trapframe->eax = 0;
    strncpy(child->name, thisp->name, sizeof(thisp->name));

    lock(&ptable.lk);
    child->state = PROC_READY;
    unlock(&ptable.lk);

    return child->pid;
}


/*! Exit the current process. On exit the control is immediately
 *  transferred to scheduler through `sched()`. The exited process remains
 *  in `PROC_ZOMBIE` state until its parent calls wait, which clean up
 *  the zombie process.
 * */
void exit() {
    Process *thisp = this_proc();
    if (thisp == proc_init1)
        panic("init 1 is exiting");

    lock(&ptable.lk);

    wakeup_unlocked(thisp->parent);

    // pass its children to init

    for (Process *p = ptable.t; p < &ptable.t[NPROC]; ++p) {
        if (p->parent == thisp) {
            p->parent = proc_init1;
            if (p->state == PROC_ZOMBIE) {
                wakeup_unlocked(proc_init1);
            }
        }
    }

    thisp->state = PROC_ZOMBIE;
    sched();
    panic("zombie exit");
}


/*! Wait for child process to exit. Return -1 if the process has no child */
int wait() {
    Process *thisp = this_proc();
    bool haskids;
    lock(&ptable.lk);

    for (;;) {
        haskids = false;
        for (Process *p = ptable.t; p < &ptable.t[NPROC]; ++p) {
            if (p->parent == thisp) {
                haskids = true;
                if (p->state == PROC_ZOMBIE) {
                    int pid = p->pid;
                    deallocate_process(p);
                    unlock(&ptable.lk);
                    return pid;
                }
            }
        }

        if (!haskids) {
            unlock(&ptable.lk);
            return -1;
        }

        if (thisp->killed) {
            unlock(&ptable.lk);
            return -1;
        }

        // wait for child to exist
        sleep(thisp, &ptable.lk);
    }
}


/* Give away the control back to the scheduler */
void sched() {
    Process *p = this_proc();

    if(this_cpu()->ncli != 1)
        panic("sched: locks");

    if(p->state == PROC_RUNNING)
        panic("sched: running");

    if(readeflags() & FL_IF)
        panic("sched: interruptible");

    int int_on = this_cpu()->int_on;
    swtch(&p->context, this_cpu()->scheduler);
    this_cpu()->int_on = int_on;
}


/*! Release lock and sleep on `chan`. Acquire the lock when wake up.
 *  Sleep modifies
 *  @chan  identify the channel the process is being slept. Usually
 *         it's the address of the lock.
 *  @lk    the lock the process
  * */
void sleep(void *chan, SpinLock *lk) {
    Process *p = this_proc();
    if (p == 0)
        panic("sleep");

    if (lk == 0)
        panic("sleep: lk");

    if (lk != &ptable.lk) {
        lock(&ptable.lk); // ptable lock to protect proc state
        unlock(lk);
    }

    p->chan = chan;
    p->state = PROC_SLEEPING;

    sched();

    p->chan = 0;

    if (lk != &ptable.lk) {
        unlock(&ptable.lk);
        lock(lk);
    }
}


/*! Walke up all sleeping processes sleep on `chan` */
void wakeup_unlocked(void *chan) {
    Process *p;

    for (int i = 0; i < NPROC; ++i) {
        p = &ptable.t[i];
        if (p->state == PROC_SLEEPING && p->chan == chan) {
            p->state = PROC_READY;
        }
    }
}


/*! Walke up all sleeping processes sleeps on `chan`. Protected
 *  by ptable lock.
 * */
void wakeup(void *chan) {
    lock(&ptable.lk);
    wakeup_unlocked(chan);
    unlock(&ptable.lk);
}


void yield() {
    lock(&ptable.lk);
    this_proc()->state = PROC_READY;
    sched();
    unlock(&ptable.lk);
}
