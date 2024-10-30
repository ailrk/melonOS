#pragma once
#include <stdbool.h>
#include "spinlock.h"


/* Long term locking */

typedef struct Mutex {
    bool        locked;
    SpinLock    lk;
    unsigned    pid;
    const char  *name;
} Mutex;


Mutex new_mutex (const char *name);
void  lock_mutex (Mutex *);
void  unlock_mutex (Mutex *);
bool  holding_mutex (Mutex *);
