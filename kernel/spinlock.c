#include "spinlock.h"
#include "err.h"
#include "i386.h"
#include "ncli.h"
#include "proc.h"


SpinLock new_lock() {
    return (SpinLock) {
        .locked = 0,
        .cpu = 0
    };
}

bool holding(SpinLock *lk) {
    bool b;
    push_cli();
    b = lk->locked && lk->cpu == this_cpu();
    pop_cli();
    return b;
}

void lock(SpinLock *lk) {
    push_cli();
    if (holding(lk))
        panic("lock");

    while(xchg(&lk->locked, 1) != 0);

    __sync_synchronize();

    lk->cpu = this_cpu();
    pop_cli();
}

void unlock(SpinLock *lk) {
    push_cli();
    if (!holding(lk))
        panic("unlock");

    lk->cpu = 0;
    __sync_synchronize();
    __asm__ volatile("movl $0, %0" : "+m" (lk->locked) : );

    pop_cli();
}
