#pragma once
#include <stdbool.h>
#include "process/pdefs.h"
#include "process/spinlock.h"


typedef struct PTable {
    SpinLock lk;
    Process  t[NPROC];
} PTable;


void     ptable_init();
void     init_pid1();
CPU     *this_cpu();
Process *this_proc();
Process *allocate_process();
void     deallocate_process_unlocked(Process *p);
int      grow_process(int);
