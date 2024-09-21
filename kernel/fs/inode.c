#include "fs/fdefs.h"


typedef struct ITable {
    SpinLock lk;
    Inode inodes[NINODE];
} ITable;



/*! Get stat from inode */
void inode_stat(const Inode *ino, Stat *stat) {
    stat->dev = ino->dev;
    stat->inum = ino->inum;
    stat->nlink = ino->dinode.nlink;
    stat->size = ino->dinode.size;
}
