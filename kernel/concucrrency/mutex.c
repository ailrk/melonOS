#include "mutex.h"
#include "spinlock.h"
#include "process/proc.h"


Mutex new_mutex(const char *name) {
    Mutex slk;
    slk.lk     = new_lock("mutex.lk");
    slk.locked = false;
    slk.name   = name;
    slk.pid    = 0;
    return slk;
}


/*! Is the current process holding the mutex? */
bool holding_mutex(Mutex *slk) {
    bool r;
    lock(&slk->lk);
    r = slk->locked && slk->pid == this_proc()->pid;
    unlock(&slk->lk);
    return r;
}


/*! Try acquire the mutex. If the lock is already acquired
 *  then put the current process on sleep.
 * */
void lock_mutex(Mutex *slk) {
    lock(&slk->lk);
    while (slk->locked) {
        sleep(slk, &slk->lk);
    }

    slk->locked = 1;
    slk->pid    = this_proc()->pid;
    unlock(&slk->lk);
}


/*! Release the mutex . Wake up all other processes sleeping on the mutex.
 * */
void unlock_mutex(Mutex *slk) {
    lock(&slk->lk);
    slk->locked = 0;
    slk->pid    = 0;
    wakeup(slk);
    unlock(&slk->lk);
}
