#include "fs/fdefs.h"
#include "spinlock.h"


typedef struct ITable {
    SpinLock lk;
    Inode    inodes[NINODE];
} ITable;


ITable itable;


void  inode_init(DevNum dev) {
    itable.lk = new_lock("itable.lk");
}


/*! Get stat from inode */
void inode_stat(const Inode *ino, Stat *stat) {
    stat->dev   = ino->dev;
    stat->inum  = ino->inum;
    stat->nlink = ino->dinode.nlink;
    stat->size  = ino->dinode.size;
}
