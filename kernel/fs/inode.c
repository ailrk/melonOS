#include <stddef.h>
#include "defs.h"
#include "driver/vga.h"
#include "process/spinlock.h"
#include "fs/fdefs.h"


typedef struct ICache {
    SpinLock lk;
    Inode    inodes[NINODE];
} ICache;


ICache            icache;
extern SuperBlock super_block;
extern Dev        devices[NDEV];


static const size_t inode_per_block = BSIZE / sizeof(DInode);


inline static blockno inode_block(inodenum inum) {
    return inum / inode_per_block + super_block.inodestart;
}


void  inode_init() {
    vga_printf("[\033[32mboot\033[0m] inode...");
    icache.lk = new_lock("icache.lk");
    vga_printf("\033[32mok\033[0m\n");
}


/*! Get an inode from icache. If the inode is not cached, allocate
 *  a inode cache. This function does not read from the disk.
 *  Return 0 if there is not enough slots.
 * */
Inode *icache_get(devnum dev, inodenum inum) {
    for (unsigned i = 0; i < sizeof(icache.inodes); ++i) {
        Inode *ino = &icache.inodes[i];
        if (ino->nref >= 0 && ino->dev == dev && ino->inum == inum) {
            ino->nref++;
            return ino;
        }

        if (ino->nref == 0) {
            ino->nref = 1;
            ino->dev  = dev;
            ino->inum = inum;
            ino->read = false;
            return ino;
        }
    }

    return 0;
}


/*! Increment the reference count for ino */
Inode *inode_dup(Inode *ino) {
    ino->nref++;
    return ino;
}


/*! Drop reference count of an inode. If the reference count drops to 0 and
 * link count is 0, then `inode_drop` will free the disk space then
 * the ino is free to reuse.
 * TODO implement disk operation
 * */
void inode_drop(Inode *ino) {
    ino->nref--;
}


/*! Read data from inode.
 *  @return number of bytes read. Return -1 if failed.
 * TODO implement disk operation
 * */
int inode_read(Inode *ino, char *tgt, unsigned n) {
    if (ino->d.type == F_DEV) {
        if (ino->d.major < 0 || ino->d.major > NDEV) {
            return -1;
        }

        if (!devices[ino->d.major].read) {
            return -1;
        }

        return devices[ino->d.major].read(ino, tgt, n);
    }
    return -1;
}

/*! Write data from inode.
 *  @return number of bytes written. Return -1 if failed.
 * TODO implement disk operation
 * */
int inode_write(Inode *ino, char *tgt, unsigned n) {
    if (ino->d.type == F_DEV) {
        if (ino->d.major < 0 || ino->d.major > NDEV) {
            return -1;
        }

        if (!devices[ino->d.major].write) {
            return -1;
        }

        return devices[ino->d.major].write(ino, tgt, n);
    }
    return -1;
}


/*! Get stat from inode */
void inode_stat(const Inode *ino, Stat *stat) {
    stat->dev   = ino->dev;
    stat->inum  = ino->inum;
    stat->nlink = ino->d.nlink;
    stat->size  = ino->d.size;
}
