#include "driver/vga.h"
#include "spinlock.h"
#include "fs/fdefs.h"
#include "fs/block.h"


typedef struct ITable {
    SpinLock lk;
    Inode    inodes[NINODE];
} ITable;


ITable itable;


void  inode_init() {
    vga_printf("[\033[32mboot\033[0m] inode...");
    itable.lk = new_lock("itable.lk");
    vga_printf("\033[32mok\033[0m\n");
}


/*! Get stat from inode */
void inode_stat(const Inode *ino, Stat *stat) {
    stat->dev   = ino->dev;
    stat->inum  = ino->inum;
    stat->nlink = ino->dinode.nlink;
    stat->size  = ino->dinode.size;
}
