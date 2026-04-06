#pragma once
#include "process/spinlock.h"


void     ptable_init();
void     init_pid1();
int      grow_process(int n);
int      fork();
void     exit();
int      wait();
void     sleep(void *chan, SpinLock *lk);
void     wakeup_unlocked(void *chan);
void     wakeup(void *chan);
void     yield();
void     sched();
CPU     *this_cpu();
Process *this_proc();
void     scheduler();
