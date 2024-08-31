#pragma once


#include <stdint.h>
#include <stdbool.h>
#include "gdt.h"
#include "mmu.h"

/* Per CPU state */

typedef struct CPU {
    GDTRecord   gdtr;
    GDTEntry    gdt[NSEGS];
    volatile bool   started;
    volatile bool   interrupt_enabled;
} CPU;


typedef struct Process {
    uint32_t        size;        // size of process memory
    PDE*            page_table;  // per process page table
    char*           kstack;      // bottom of kernel stack for this process
    uint32_t        pid;         // process id
    char*           name[16];    // name of the process
} Process;


typedef enum ProcState {
    PROC_CREATED,
    PROC_TERMINATED,
    PROC_RUNNING, 
    PROC_READY,
    PROC_BLOCKED,
    PROC_SWP_WAITING,            // swapped out and waiting 
    PROC_SWP_BLOCKED,            // swapped out and blocked
} ProcState;
