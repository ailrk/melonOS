#include <stddef.h>
#include "bcache.h"
#include "block.h"
#include "defs.h"
#include "driver/vga.h"
#include "err.h"
#include "fdefs.fwd.h"
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


void inode_init() {
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


/* Return the blockno of the nth block of inode. Allocate blocks if necessary.
 * */
blockno inode_map(Inode *ino, unsigned nth) {
    if (nth < NDIRECT) {
        blockno blockno = 0;
        if ((blockno = ino->d.addrs[nth]) == 0) {
            blockno = block_alloc(ino->dev);
        }
        return blockno;
    }

    if (nth >= NDIRECT) { // singly indirect
        blockno ptrsno  = 0;
        blockno offset  = nth - NDIRECT;
        blockno blockno = 0;
        if ((ptrsno = ino->d.addrs[nth]) == 0) {
            ptrsno = block_alloc(ino->dev);
        }
        BNode *blockptrs = bcache_read(ino->dev, ptrsno, false);
        if ((blockno = ((unsigned *)blockptrs->cache)[offset]) == 0) {
            blockno = block_alloc(ino->dev);
        }
        bcache_release(blockptrs);
        return blockno;
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
 *  @ino    Inode
 *  @buf    the buffer read into
 *  @offest cursor offset
 *  @sz     read size
 *  @return number of bytes read. Return -1 if failed.
 * TODO implement disk operation
 * */
int inode_read(Inode *ino, char *buf, unsigned offset, unsigned sz) {
    switch(ino->d.type) {
    case F_DEV:
        if (ino->d.major < 0 || ino->d.major > NDEV) {
            return -1;
        }
        if (!devices[ino->d.major].read) {
            return -1;
        }
        return devices[ino->d.major].read(ino, buf, sz);
    case F_DIR:
        panic("inode_read: dir not implemented");
        return -1;
    case F_FILE:
        panic("inode_read: file not implemented");
        return -1;
    }
}

/*! Write data from inode.
 *  @ino    Inode
 *  @buf    the buffer write from
 *  @offest cursor offset
 *  @sz     read size
 *  @return number of bytes written. Return -1 if failed.
 * TODO implement disk operation
 * */
int inode_write(Inode *ino, const char *buf, unsigned offset, unsigned n) {
    switch (ino->d.type) {
    case F_DEV:
        if (ino->d.major < 0 || ino->d.major > NDEV) {
            return -1;
        }
        if (!devices[ino->d.major].write) {
            return -1;
        }
        return devices[ino->d.major].write(ino, buf, n);
    case F_DIR:
        panic("inode_write: dir not implemented");
        return -1;
    case F_FILE:
        panic("inode_write: file not implemented");
        return -1;
    }
}


/*! Get stat from inode */
void inode_stat(const Inode *ino, Stat *stat) {
    stat->dev   = ino->dev;
    stat->inum  = ino->inum;
    stat->nlink = ino->d.nlink;
    stat->size  = ino->d.size;
}
