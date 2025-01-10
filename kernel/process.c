#include "defs.h"
#include "fdefs.fwd.h"
#include "string.h"
#include "err.h"
#include "process.h"
#include "process/proc.h"
#include "process/pdefs.h"
#include "memory/vmem.h"
#include "driver/vga.h"
#include "fs/file.h"


extern void swtch (Context **save, Context *load);
extern PTable   ptable;
extern Process *proc_init1;


void process_init () {
    ptable_init();
    init_pid1();
}


/*! Fork a process
 *  @return  return 0 on child process, pid of the child process on parent. -1
 *           if failed.
 * */
int fork () {
    Process *child;
    Process *thisp = this_proc ();

    if ((child = allocate_process ()) == 0) {
        return -1;
    }

    if ((child->pgdir = uvm_copy (thisp->pgdir, thisp->size)) == 0) {
        deallocate_process (child);
        return -1;
    }

    child->size           = thisp->size;
    child->parent         = thisp;
    *child->trapframe     = *thisp->trapframe;
    child->trapframe->eax = 0;

    for (int i = 0; i < NFILE; ++i) {
        File *f = thisp->file[i];
        if (f) {
            child->file[i] = file_dup(f);
        }
    }

    strncpy (child->name, thisp->name, sizeof(thisp->name));

    lock (&ptable.lk);
    child->state = PROC_READY;
    unlock (&ptable.lk);

    return child->pid;
}


/*! Exit the current process. On exit the control is immediately
 *  transferred to scheduler through `sched()`. The exited process remains
 *  in `PROC_ZOMBIE` state until its parent calls wait, which clean up
 *  the zombie process.
 * */
void exit () {
    Process *thisp = this_proc ();
    if (thisp == proc_init1)
        panic ("init 1 is exiting");

    lock (&ptable.lk);

    wakeup_unlocked (thisp->parent);

    // pass its children to init
    for (Process *p = ptable.t; p < &ptable.t[NPROC]; ++p) {
        if (p->parent == thisp) {
            p->parent = proc_init1;
            if (p->state == PROC_ZOMBIE) {
                wakeup_unlocked (proc_init1);
            }
        }
    }

    thisp->state = PROC_ZOMBIE;
    sched ();
    panic ("zombie exit");
}


/*! Wait for child process to exit. Return -1 if the process has no child */
int wait () {
    Process *thisp = this_proc ();
    bool haskids;
    lock (&ptable.lk);

    for (;;) {
        haskids = false;
        for (Process *p = ptable.t; p < &ptable.t[NPROC]; ++p) {
            if (p->parent == thisp) {
                haskids = true;
                if (p->state == PROC_ZOMBIE) {
                    int pid = p->pid;
                    deallocate_process (p);
                    unlock (&ptable.lk);
                    return pid;
                }
            }
        }

        if (!haskids) {
            unlock (&ptable.lk);
            return -1;
        }

        if (thisp->killed) {
            unlock (&ptable.lk);
            return -1;
        }

        // wait for child to exist
        sleep (thisp, &ptable.lk);
    }
}


/* Give away the control back to the scheduler */
void sched () {
    Process *p = this_proc ();

    if (this_cpu()->ncli != 1)
        panic("sched: locks");

    if (p->state == PROC_RUNNING)
        panic ("sched: running");

    if (readeflags() & FL_IF)
        panic ("sched: interruptible");

    int int_on = this_cpu()->int_on;
    swtch (&p->context, this_cpu()->scheduler);
    this_cpu()->int_on = int_on;
}


/*! Release lock and sleep on `chan`. Acquire the lock when wake up.
 *  Sleep modifies
 *  @chan  identify the channel the process is being slept. Usually
 *         it's the address of the lock.
 *  @lk    the lock the process
  * */
void sleep (void *chan, SpinLock *lk) {
    Process *p = this_proc ();
    if (p == 0)
        panic ("sleep");

    if (lk == 0)
        panic ("sleep: lk");

    if (lk != &ptable.lk) {
        lock (&ptable.lk); // ptable lock to protect proc state
        unlock (lk);
    }

    p->chan  = chan;
    p->state = PROC_SLEEPING;

    sched ();

    p->chan = 0;

    if (lk != &ptable.lk) {
        unlock (&ptable.lk);
        lock (lk);
    }
}


/*! Walke up all sleeping processes sleep on `chan` */
void wakeup_unlocked (void *chan) {
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
void wakeup (void *chan) {
    lock (&ptable.lk);
    wakeup_unlocked (chan);
    unlock (&ptable.lk);
}


void yield () {
    lock (&ptable.lk);
    this_proc()->state = PROC_READY;
    sched ();
    unlock (&ptable.lk);
}


/*! The scheduler next returns. CPU loops over the `ptable` and look
 *  for the next process to run. The loop will pick the first ready
 *  process, switch to run the process, wait until the process switch
 *  back to the scheduler.
 * */
void scheduler () {
    vga_printf ("[\033[32mboot\033[0m] scheduler...\n");
    CPU *cpu = this_cpu ();
    cpu->proc = 0;
    for (;;) {
        sti (); // force enable interrupt
        lock (&ptable.lk);
        for (Process *p = ptable.t; p < ptable.t + NPROC; ++p) {
            if (p->state != PROC_READY)
                continue;
            cpu->proc = p;
            uvm_switch (p);
            p->state = PROC_RUNNING;
            swtch (&cpu->scheduler, p->context);
            kvm_switch ();
            cpu->proc = 0;
        }
        unlock (&ptable.lk);
    }
}
