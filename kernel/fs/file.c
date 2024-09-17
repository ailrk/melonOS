#include "fs/file.h"
#include "proc.h"
#include "err.h"
#include "spinlock.h"


Dev devs[NDEV];

typedef struct FTable {
    SpinLock lk;
    File t[NFILE];
} FTable;

FTable ftable;


void file_init() {
    ftable.lk = new_lock();
}



/*! Allocate a file from the ftable
 *  @return Newly allocated file. 0 if failed.
 * */
File *allocate_file() {
    lock(&ftable.lk);
    File *f = 0;
    for (int i = 0; i < NFILE; ++i) {
        *f = ftable.t[i];
        if (f->nref == 0) {
            f->nref = 1;
            return f;
        }
    }
    unlock(&ftable.lk);
    return 0;
}


/*! Increment the reference count */
File *dup_file(File *f) {
    lock(&ftable.lk);
    if (f->nref < 1)
        panic("dup_file");
    f->nref++;
    unlock(&ftable.lk);
    return 0;
}



/*! Decrement the reference count. When `nref` = 0
 *  deallocate resources.
 * */
void close_file(File *f) {

}
