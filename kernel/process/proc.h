#pragma once
#include <stdbool.h>
#include "concucrrency/spinlock.h"
#include "process/pdefs.h"


void     ptable_init();
void     init_pid1();
CPU *    this_cpu();
Process *this_proc();
void     scheduler();
int      fork();
void     exit();
int      wait();
void     sleep(void *chan, SpinLock *lk);
void     wakeup_unlocked(void *chan);
void     wakeup(void *chan);
void     yield();
void     sched();
