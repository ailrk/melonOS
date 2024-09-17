#pragma once

#include <stdbool.h>
#include "spinlock.h"


/* Long term locking */

typedef struct SleepLock {
    bool        locked;
    SpinLock    lk;
    int         pid;
    const char  *name;
} SleepLock;


SleepLock new_sleeplock(const char *name);
void lock_sleep(SleepLock *);
void unlock_sleep(SleepLock *);
void holding_sleep(SleepLock *);
