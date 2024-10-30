#pragma once
#include "process/pdefs.h"


typedef struct SpinLock {
    unsigned    locked;
    CPU *       cpu;
    const char *name;
} SpinLock;


SpinLock new_lock (const char *name);
void     lock (SpinLock *);
void     unlock (SpinLock *);
