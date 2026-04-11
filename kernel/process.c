#include "debug.h"
#include "defs.h"
#include "fdefs.fwd.h"
#include "string.h"
#include "err.h"
#include "print.h"
#include "spinlock.h"
#include "process.h"
#include "process/pdefs.h"
#include "memory/vmem.h"
#include "memory/palloc.h"
#include "memory/vmem.h"
#include "trap/ncli.h"
#include "fs/file.h"


extern Process *proc_init1;
extern void     trapret();
extern void     swtch(Context **save, Context *load);


/* The next pid. */
int nextpid = 1;


/* Pointer to proc_init1 */
Process *proc_init1;


/* The CPU state */
CPU cpu;


/* Process table */
struct PTable {
    SpinLock lk;
    Process  t[NPROC];
} ptable;


#if DEBUG
void dump_context(const Context *c) {
    if (!c) {
        debug("CTX> no context yet\n");
        return;
    }
    debug("CTX>\n");
    debug("  edi: %#x\n", c->edi);
    debug("  esi: %#x\n", c->esi);
    debug("  ebx: %#x\n", c->ebx);
    debug("  ebp: %#x\n", c->ebp);
    debug("  eip: [<%#x>]\n", c->eip);
}


void dump_process(const Process *p) {
    if (p->pid > NPROC)
        return;

    char *state = 0;
    switch (p->state) {
        case PROC_UNUSED:   state = "unused";   break;
        case PROC_CREATED:  state = "created";  break;
        case PROC_SLEEPING: state = "sleeping"; break;
        case PROC_READY:    state = "ready";    break;
        case PROC_RUNNING:  state = "running";  break;
        case PROC_ZOMBIE:   state = "zombie";   break;
        default:            state = "?";        break;
    }
    debug("PROC>\n", p->pid, p->name, state);
    debug("  pid:   %d\n", p->pid);
    debug("  name:  %s\n", p->name);
    debug("  state: %s\n", state);
}
#endif


/* Initialize the process table */
void ptable_init() {
    ptable.lk = new_lock("ptable.lk");
}


/* Return the running cpu */
CPU *this_cpu() {
    if (readeflags() & FL_IF)
        panic ("interrupt is enabled on `this_cpu` call");

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


/* Return a forked child process to the user space */
void forkret() {
    // it's locked from scheduler.
    unlock(&ptable.lk);
}


/* Try to get the next unused process in ptable */
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


/* The process's kernel stack has the following setup:
 *
 * ..0x0
 * +-----------------------+ <- kstk
 * | ...                   |
 * +-----------------------+ <- p->context
 * | Context               |
 * | | ...                 |
 * | | eip = forkret       |
 * +-----------------------+
 * | trapret (Return Addr) |
 * +-----------------------+ <- p->trapframe
 * | TrapFrame             |
 * | | ...                 |
 * | | eip = 0 (or entry)  |
 * +-----------------------+ <- kstk + KSTACK_SZ
 *
 * Note the kernel stack size is fixed, unlike user stack which you can grow by
 * adding more pages. This is because Kernel stack is the ground truth,
 * operations on virtual memory themselves rely on it being stable.
 *
 * Before switching in to the process, swtch will pop all context except $eip.
 * When it `ret`, the control will be transferred to `eip` in this case we set
 * to `forkret.` `forkret` simply return to caller. In this case it will return
 * to `trapret`, which will further pop all registers in the trap frame.
 *
 * We set the eip in the trapframe to 0, so after `trapret` the process will
 * start running on the address 0x0.
 */
static bool setup_process_stack(Process *p) {
    // allocate and build the kernel stack
    if ((p->kstack = palloc()) == 0) {
        p->state = PROC_UNUSED;
        return false;
    }

    char *sp        = p->kstack + KSTACK_SZ;

    // Allocate TrapFrame.
    sp             -= sizeof(TrapFrame);
    p->trapframe    = (TrapFrame*)sp;

    // Setup `trapret` as the return address from `forkret`. `trapret` will
    // restore the CPU state from (TramFrame *).
    sp             -= sizeof (uintptr_t);
    *(uintptr_t*)sp = (uintptr_t)trapret;

    // Allocate Context.
    sp             -= sizeof(Context);
    p->context      = (Context*)sp;

    memset (p->context, 0, sizeof (Context));

    // swtch returns on `forkret`
    p->context->eip = (uint32_t)forkret;
    return true;
}


/* Allocate a new process and set it up to run in kernel. */
Process *allocate_process() {
    if (nextpid > NPROC)
        panic ("allocate_process: can't allocate more pids");

    lock(&ptable.lk);

    Process *p = get_unused_process();

    if (!p) {
        unlock(&ptable.lk);
        return 0;
    }

    p->state = PROC_CREATED;
    p->pid   = nextpid++;
    unlock(&ptable.lk);

    // Construct the process stack for context switching
    if (!setup_process_stack(p)) {
        return 0;
    }
    return p;
}


/* Deallocate a process.
 *
 * It tries to free all resources hold by the process. Note this is not locked,
 * it should be called in a critical section.
 * */
void deallocate_process_unlocked(Process *p) {
    p->size = 0;
    if (p->pgdir.t) {
        vmfree(p->pgdir);
        p->pgdir.t = 0;
    }
    if (p->kstack)  {
        pfree(p->kstack);
        p->kstack = 0;
    }
    p->kstack    = 0;
    p->pid       = 0;
    p->parent    = 0;
    p->state     = PROC_UNUSED;
    p->trapframe = 0;
    p->context   = 0;
    p->chan      = 0;
    p->killed    = 0;
    memset(p->name, 0, sizeof(p->name));
}


/* Setup the trapframe for the first process to create an illusion that a trap
 * occured. So if we call trapret, it will pop all registers in the trap frame
 * hence switch the control.
 * */
static void set_pid1_trapframe(Process *p) {
    memset(p->trapframe, 0, sizeof (*p->trapframe));
    p->trapframe->cs     = (SEG_UCODE << 3) | DPL_U; // set segment DPL
    p->trapframe->ds     = (SEG_UDATA << 3) | DPL_U;
    p->trapframe->es     = p->trapframe->ds;
    p->trapframe->ss     = p->trapframe->ds;
    p->trapframe->eflags = FL_IF;
    p->trapframe->esp    = PAGE_SZ;
    p->trapframe->eip    = 0; // start of the program
}


/* Initialize the first user space process. We allocate 1 page for the user
 * memory of init. The source code of init is direclty copied from the kernel's
 * .text section to the user vmem.
 *
 *   +----------------+
 *   | program data   |
 *   |    & heap      |
 *   +----------------+
 *   | user stack     |
 *   +----------------+
 *   | user data      |
 *   +----------------+ .text end at __INIT_END__
 *   | user text      |
 * 0 +----------------+ copied from __INIT_BEGIN__
 * */
void init_pid1() {
    dprintf(LOG_BOOT " init1...\n");
    Process *p;
    if ((p = allocate_process()) == 0) {
        panic ("init_pid1: failed to allocate process");
    }

    // Allocate a page to hold the pgdir for init. The kernel space is also
    // mapped into the pgdir in this process.
    if (!kvm_allocate(&p->pgdir))
        panic("init_pid1");

    extern char __INIT1_BEGIN__[];
    extern char __INIT1_END__[];

    int init1_sz = __INIT1_END__ - __INIT1_BEGIN__;

    // init itself starts with 1 page user memory.
    uvm_init1(p->pgdir, __INIT1_BEGIN__, init1_sz);
    p->size = PAGE_SZ;
    set_pid1_trapframe(p);
    strncpy(p->name, "init", sizeof(p->name));

    // Protect the state transition
    lock(&ptable.lk);
    p->state = PROC_READY;
    proc_init1 = p;
    unlock(&ptable.lk);

    dprintf(LOG_BOOT " init1 " LOG_OK "\n");
}


/* Grow process user memory by n bytes, n can be negative. */
int grow_process(int n) {
    if (n == 0)
        return 0;

    Process *p = this_proc();
    size_t sz = p->size;

    if (n > 0) {
        if ((sz = uvm_allocate(p->pgdir, sz, sz + n)) == 0) {
            return -1;
        }
    } else {
        if ((sz = uvm_deallocate(p->pgdir, sz, sz + n)) == 0)
            return -1;
    }

    p->size = sz;
    uvm_switch(p);
    return 0;
}


/* Fork a process
 * @return  return 0 on child process, pid of the child process on parent. -1
 *          if failed.
 * */
int fork() {
    Process *child;
    Process *thisp = this_proc();

    if ((child = allocate_process()) == 0) {
        return -1;
    }

    // Copy from the parent user memory.
    if (!uvm_copy (&child->pgdir, thisp->pgdir, thisp->size)) {
        deallocate_process_unlocked(child);
        return -1;
    }

    // Setup child process.
    child->size           = thisp->size;
    child->parent         = thisp;
    *child->trapframe     = *thisp->trapframe;
    child->trapframe->eax = 0; // child return 0

    // Copy parent fds
    for (int i = 0; i < NOFILE; ++i) {
        File *f = thisp->file[i];
        if (f) {
            child->file[i] = file_dup(f);
        }
    }

    strncpy(child->name, thisp->name, sizeof(thisp->name));

    // State transition needs to be protected by lock. The forked process will
    // be picked up by the scheduler.
    lock(&ptable.lk);
    child->state = PROC_READY;
    unlock(&ptable.lk);

    return child->pid;
}


/* Exit the current process. On exit the control is immediately transferred to
 * scheduler through `sched()`. The exited process remains in `PROC_ZOMBIE`
 * state until its parent calls wait, which clean up the zombie process.
 * */
void exit() {
    Process *thisp = this_proc();
    if (thisp == proc_init1)
        panic("init 1 is exiting");

    lock(&ptable.lk);

    wakeup_unlocked(thisp->parent);

    // Pass its children to init
    for (Process *p = ptable.t; p < &ptable.t[NPROC]; ++p) {
        if (p->parent == thisp) {
            p->parent = proc_init1;
            if (p->state == PROC_ZOMBIE) {
                wakeup_unlocked (proc_init1);
            }
        }
    }

    thisp->state = PROC_ZOMBIE;
    sched();
    panic("zombie exit");
}


/* Wait for child process to exit. Return -1 if the process has no child */
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
                    deallocate_process_unlocked(p);
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


/* Give away the control back to the scheduler.
 * Sched leave the orignal process state untouched. */
void sched() {
    Process *p = this_proc();

    if (!holding(&ptable.lk))
        panic("sched: ptable.lk");

    if (this_cpu()->ncli != 1)
        panic("sched: locks");

    if (p->state == PROC_RUNNING)
        panic("sched: running");

    if (readeflags() & FL_IF)
        panic("sched: interruptible");

    // We simply switch back to the scheduler.
    int int_on = this_cpu()->int_on;
    swtch(&p->context, this_cpu()->scheduler);
    this_cpu()->int_on = int_on;
}


/* Release lock and sleep on `chan`. Acquire the lock when wake up. Sleep
 * modifies
 * @chan  identify the channel the process is being slept. Usually
 *        it's the address of the lock.
 * @lk    the lock the process
  * */
void sleep(void *chan, SpinLock *lk) {
    Process *p = this_proc ();
    if (p == 0)
        panic("sleep");

    if (lk == 0)
        panic("sleep: lk");

    // Lock PTable  to protect process state Unlock the sleep lock to proceed.
    if (lk != &ptable.lk) {
        lock(&ptable.lk);
        unlock(lk);
    }

    p->chan  = chan;
    p->state = PROC_SLEEPING;

    sched();

    p->chan = 0;

    // Reverse the lock.
    if (lk != &ptable.lk) {
        unlock(&ptable.lk);
        lock(lk);
    }
}


/* Walke up all sleeping processes sleep on `chan` */
void wakeup_unlocked(void *chan) {
    Process *p;

    for (int i = 0; i < NPROC; ++i) {
        p = &ptable.t[i];
        if (p->state == PROC_SLEEPING && p->chan == chan) {
            p->state = PROC_READY;
        }
    }
}


/* Walke up all sleeping processes sleeps on `chan`. Protected by ptable lock. */
void wakeup(void *chan) {
    lock(&ptable.lk);
    wakeup_unlocked(chan);
    unlock(&ptable.lk);
}


/* Like sched but set the process state to ready so it will be picked up by the
 * scheduler later.
 * */
void yield() {
    lock (&ptable.lk);
    this_proc()->state = PROC_READY;
    sched();
    unlock(&ptable.lk);
}


/* The scheduler.
 *
 * CPU loops over the `ptable` and look for the next process to run.
 * The loop will pick the first ready process, switch to run the
 * process, wait until the process switch back to the scheduler.
 *
 * Context switching saga:
 *
 * Imagine a process running in user mode, kernel stack is empty.
 *
 * * P1 User Mode -> Interrupt -> TrapFrame -> P1 Kernel Mode:
 *     Timer interrupt kicks in, the process is switched into the kernel.
 *     `trapgo` and the CPU together builds the `TrapFrame`. Then `trap`
 *     runs, which eventually to `yield` to the scheduler.
 *
 * * P1 Kernel Mode -> swtch -> Scheduler:
 *     `yield` wil run `swtch`, which stores the current kernel Context,
 *      The process kernel stack looks like this:
 *
 *      +-----------------------+ <- kstk
 *      | ...                   |
 *      +-----------------------+ <- p->context
 *      | Context               |
 *      | | ...                 |
 *      | | eip = forkret       |
 *      +-----------------------+
 *      | trapret (Return Addr) |
 *      +-----------------------+ <- p->trapframe
 *      | TrapFrame             |
 *      | | ...                 |
 *      | | eip = 0 (or entry)  |
 *      +-----------------------+ <- kstk + KSTACK_SZ
 *
 *      Then `switch` will restore the this_cpu()->scheduler, then return
 *      to this_cpu()->scheduler.eip. This resumes the scheduler. We are
 *      using the gobal kernels stack now.
 *
 * * Scheduler -> swtch -> P2 Kernel Mode:
 *     The scheduler look for the next READY proess, then it tries to
 *     `swtch` to it. `swtch` saves the current Context for scheduler
 *     and restore the p->context of the READY process, return to
 *     p->context->eip. Now we are in Kernel mode of the process.
 *     $eip points to `forkret`, which returns to the next address `trapret`.
 *
 * * P2 Kernel Mode -> trapret -> iret -> P2 User Mode:
 *     The full trapframe that contains the state of this process is still
 *     in the stack, `trapret` recover the state and resume the user space
 *     program.
 *
 * Another cycle begins.
 *
 * */
void scheduler() {
    dprintf(LOG_BOOT " scheduler...\n");
    CPU *cpu = this_cpu ();
    cpu->proc = 0;
    for (;;) {
        sti(); // force enable interrupt
        lock(&ptable.lk);
        for (Process *p = ptable.t; p < ptable.t + NPROC; ++p) {
            if (p->state != PROC_READY)
                continue;

#if DEBUG && DEBUG_PROC
            debug("====== SWTCH to pid %d =====\n", p->pid);
            dump_process(p);
#endif
            /* At this point, we are using the global
             * kernel page table, on the global kernel stack.
             * This kernel stack is dedicated for the scheduler.
             *
             * Since we found a ready process, next we need to
             * switch to it's page table.
             */

            // Set current process to the ready process.
            cpu->proc = p;

            /* First switch to user page table.
             * Now we have access to the user kernel stack.
             */
            uvm_switch(p);

            // Set the state to running.
            p->state = PROC_RUNNING;

            /* Context switching.
             *
             * The CPU will enter the user space and start to
             * execute the process.
             *
             * First, The scheduler's Context is stored into
             * cpu->scheduler.
             *
             * Then, p->context is restored, swtch returns
             * to $eip in p->context.
             */
            swtch(&cpu->scheduler, p->context);

            /*
             * Now The process is running ...
             */

            /* Now Process yield back to the scheduler.
             * The first thing t do is to swtich
             * back to kernel only page table.
             */

#if DEBUG && DEBUG_PROC
            debug("====== SWTCH from pid %d =====\n", p->pid);
#endif
            kvm_switch();

            /* Clear the current process, the
             * scheduler will pick up a ready process
             * in the next iteration.
             */
            cpu->proc = 0;
        }
        unlock(&ptable.lk);
    }
}
