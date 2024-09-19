#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include "gdt.h"
#include "mmu.h"
#include "trap.h"
#include "fs/fdefs.h"


#define NPROC    64 // max number of processes
#define NOFILE   32 // max number of open files per process


typedef enum ProcState {
    PROC_UNUSED = 0,
    PROC_CREATED,
    PROC_SLEEPING,
    PROC_READY,
    PROC_RUNNING,
    PROC_ZOMBIE
} ProcState;


// In x86 convention %eax, %ecx, %edx are caller saved register so we
// don't need to save them in context. Segment registers are always the
// same so we don't need to save them either.
// For context swithcing we only need to save the following general
// purpose registers.
typedef struct Context {
  uint32_t          edi;
  uint32_t          esi;
  uint32_t          ebx;
  uint32_t          ebp;
  uint32_t          eip;
} Context;


typedef struct Process {
    uint32_t        size;         // size of process memory
    PD              pgdir;        // per process page table
    char *          kstack;       // bottom of kernel stack for this process
    unsigned int    pid;          // process id
    unsigned int    ppid;         // parent pid
    ProcState       state;        // process state
    TrapFrame *     trapframe;    // process trapframe
    Context *       context;      // process context
    void *          chan;         // sleep on chan if it's not zero
    bool            killed;       // is process killed
    File            file[NOFILE]; // files
    char            name[16];     // name of the process
} Process;


/* Per CPU state */
typedef struct CPU {
    GDTRecord       gdtr;
    GDTEntry        gdt[NSEGS];
    TaskState       ts;
    Context *       scheduler; // Scheduler context. Points to kernel stack.
    volatile bool   started;
    bool            int_on; // was int enabled when ncli = 0
    int             ncli;   // levels of pushcli
    Process *       proc;
} CPU;
