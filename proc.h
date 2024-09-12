#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gdt.h"
#include "mmu.h"
#include "trap.h"


#define NPROC    64


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
    uint32_t        size;        // size of process memory
    PDE *           page_table;  // per process page table
    char *          kstack;      // bottom of kernel stack for this process
    uint32_t        pid;         // process id
    uint32_t        ppid;        // parent pid
    ProcState       state;       // process state
    TrapFrame *     trapframe;   // process context
    Context *       context;     // process context
    void *          chan;        // sleep on chan if it's not zero
    bool            killed;      // is process killed
    char            name[16];    // name of the process
} Process;


/* Per CPU state */
typedef struct CPU {
    GDTRecord       gdtr;
    GDTEntry        gdt[NSEGS];
    volatile bool   started;
    bool            int_on; // was int enabled when ncli = 0
    int             ncli;   // levels of pushcli
    Process *       proc; 
} CPU;


void init_pid1();
CPU *this_cpu();
Process *this_proc();
