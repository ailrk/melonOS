#include "err.h"
#include "spinlock.h"
#include "fs/file.h"

/* file descriptor */


typedef struct FTable {
    SpinLock lk;
    File     t[NFILE];
} FTable;


FTable ftable;


void ftable_init() {
    ftable.lk = new_lock("ftable.lk");
}


/*! Allocate a file from the ftable
 *  @return Newly allocated file. 0 if failed.
 * */
File *allocate_file() {
    File *f = 0;
    for (int i = 0; i < NFILE; ++i) {
        *f = ftable.t[i];
        if (f->nref == 0) {
            f->nref = 1;
            return f;
        }
    }
    return 0;
}


/*! Increment the reference count */
File *dup_file(File *f) {
    if (f->nref < 1)
        panic("dup_file");
    f->nref++;
    return 0;
}
