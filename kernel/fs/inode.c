#include "fs/fdefs.h"


/*! Get stat from inode */
void stat_ino(const Inode *i, Stat *stat) {
    stat->dev = i->dev;
    stat->ino = i->ino;
    stat->nlink = i->dinode.nlink;
    stat->size = i->dinode.size;
}
