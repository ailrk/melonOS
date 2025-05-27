#include "spinlock.h"
#include "err.h"
#include "i386.h"
#include "ncli.h"
#include "process/proc.h"

/* Simple busy waiting spinlock */


SpinLock new_lock(const char *name) {
    return (SpinLock) {
        .locked = 0,
        .cpu    = 0,
        .name   = name
    };
}


bool holding(SpinLock *lk) {
    bool b;
    push_cli();
    b = lk->locked && lk->cpu == this_cpu();
    pop_cli();
    return b;
}


/* Disable interrupt for the entire critical section to avoid
 * deadlock.
 * e.g If an interrupt handler also acquire the same lock
 * the lock will never be released.
 * */
void lock(SpinLock *lk) {
    push_cli();
    if (holding(lk))
        panic("lock");

    while(xchg((uint32_t*)&lk->locked, 1) != 0);

    __sync_synchronize();

    lk->cpu = this_cpu();
}


void unlock(SpinLock *lk) {
    if (!holding(lk))
        panic("unlock");

    lk->cpu = 0;
    __sync_synchronize();
      // The xchg is atomic.
    while(xchg(&lk->locked, 0) == 0);
    pop_cli();
}
