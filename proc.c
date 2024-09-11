#include "proc.h"
#include "defs.h"
#include "palloc.h"
#include "trap.h"
#include "tty.h"
#include "string.h"

CPU cpu;


typedef struct ProcTable {
    Process tb[NPROC];
} ProcTable;

ProcTable ptable;
int nextpid = 0;



/*! Allocate a new process and set it up to run in kernel. */
static Process *allocate_process() {
    Process *p;
    bool found = false;

    for (int i = 0; i < NPROC; ++i) {
        p = &ptable.tb[i];
        if (p->state == PROC_UNUSED) {
            found = true;
            break;
        }
    }

    if (!found) return 0;
    if (p->state == PROC_UNUSED) return 0;

    p->state = PROC_CREATED;
    p->pid = nextpid++;

    if ((p->kstack = palloc()) == 0) {
        p->state = PROC_UNUSED;
        return 0;
    }

    char *sp = p->kstack + KSTACK_SZ;

    sp -= sizeof(TrapFrame);
    p->trapframe = (TrapFrame*)sp;

    sp -= sizeof(Context);
    p->context = (Context*)sp;

    memset(p->context, 0, sizeof(Context));

    return p;
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

    tty_printf("%d %s %s", p->pid, state, p->name);
}



void dump_processes() {
    for (int i = 0; i < NPROC; ++i) {
        Process p = ptable.tb[i];
    }
}
