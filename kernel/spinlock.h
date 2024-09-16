#pragma once

#include "process/proc.h"


typedef struct SpinLock {
    unsigned int locked;
    CPU *cpu;
} SpinLock;


SpinLock new_lock();
void lock(SpinLock *);
void unlock(SpinLock *);
