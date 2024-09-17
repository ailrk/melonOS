#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "gdt.h"
#include "mmu.h"
#include "spinlock.h"
#include "trap.h"
#include "process/pdefs.h"
#include "fs/file.h"


void ptable_init();
void init_pid1();
CPU *this_cpu();
Process *this_proc();
void scheduler();
int fork();
int wait();
void sleep(void *chan, SpinLock *lk);
void wakeup(void *chan);
void yield();
void exit();
