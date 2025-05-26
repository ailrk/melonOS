#include "err.h"
#include "fdefs.fwd.h"
#include "spinlock.h"
#include "fs/inode.h"
#include "fs/file.h"

/* file descriptor */


typedef struct FTable {
    SpinLock lk;
    File     t[NFILE];
} FTable;


FTable ftable;


void ftable_init () {
    ftable.lk = new_lock ("ftable.lk");
}


/*! Allocate a file from the ftable
 *  @return newly allocated file. 0 if failed.
 * */
File *file_allocate () {
    File *f;
    for (int i = 0; i < NFILE; ++i) {
        f = &ftable.t[i];
        if (f->nref == 0) {
            f->nref = 1;
            return f;
        }
    }
    return 0;
}


/*! Increment the reference count */
File *file_dup (File *f) {
    if (f->nref < 1)
        panic ("dup_file");
    f->nref++;
    return 0;
}

/*! Close a file  */
void file_close (File *f) {
    if (f->nref < 1)
        panic ("file_close");

    if (--f->nref > 0)
        return;

    // no more references, close the file.
    File fcpy = *f;
    f->nref = 0;
    f->type = FD_NONE;

    switch (fcpy.type) {
        case FD_INODE:
            inode_drop (fcpy.ino);
            return;
        default:
            return;
    }
}

/*! Read file from file descriptor  */
int file_read (File *f, char *buf, int n) {
    int rd;

    if (!f->readable) return -1;

    switch (f->type) {
    case FD_NONE:
        panic("file read");
    case FD_PIPE:
        panic("file_read: pipe not supported");
    case FD_INODE:
        if ((rd = inode_read (f->ino, buf, f->offset, n)) > 0)
            f->offset += rd;
        return rd;
    }
    return -1;
}


/*! Write to file descriptor  */
int file_write (File *f, const char *buf, int n) {
    int wt;

    if (!f->writable) return -1;

    switch (f->type) {
    case FD_NONE:
        panic ("file read");
    case FD_PIPE:
        panic ("file_read: pipe not supported");
    case FD_INODE:
        if ((wt = inode_write (f->ino, buf, f->offset, n)))
            f->offset += wt;
        return wt;
    }
    return -1;
}
