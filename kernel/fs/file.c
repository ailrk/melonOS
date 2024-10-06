#include "err.h"
#include "spinlock.h"
#include "fs/inode.h"
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
File *file_allocate() {
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
File *file_dup(File *f) {
    if (f->nref < 1)
        panic("dup_file");
    f->nref++;
    return 0;
}


/*! Read file from file descriptor  */
int file_read(File *f, char *buf, int n) {
    int r;

    if (!f->readable) {
        return -1;
    }

    if (f->type == FD_NONE) {
        return -1;
    }

    if (f->type == FD_INODE) {
        if ((r = inode_read(f->ino, buf, f->offset, n)) > 0)
            f->offset += r;
        return r;
    }

    if (f->type == FD_PIPE) {
        panic("read_file: pipe not supported");
    }

    return -1;
}


/*! Write to file descriptor  */
int file_write(File *f, const char *buf, int n) {
    int r;

    if (!f->writable) {
        return -1;
    }

    if (f->type == FD_INODE) {
        if ((r = inode_write(f->ino, buf, f->offset, n)))
            f->offset += r;
        return r;
    }

    panic("read_file");
    return -1;
}
