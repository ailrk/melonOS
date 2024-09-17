#include "sleeplock.h"
#include "spinlock.h"
#include "process/proc.h"


SleepLock new_sleeplock(const char *name) {
    SleepLock slk;
    slk.lk = new_lock("sleeplk.spinlk");
    slk.locked = false;
    slk.name = name;
    slk.pid = 0;
    return slk;
}
