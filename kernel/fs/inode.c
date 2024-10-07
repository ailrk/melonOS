#include <stddef.h>
#include "stdlib.h"
#include "string.h"
#include "bcache.h"
#include "block.h"
#include "defs.h"
#include "err.h"
#include "driver/vga.h"
#include "process/spinlock.h"
#include "fs/fdefs.fwd.h"
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
Inode *inode_get(devnum dev, inodenum inum) {
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



/*! Flush in memory inode cache to disk. Needs to be called everytime inode field is updated. */
void inode_flush(Inode *ino) {
    BNode *b = bcache_read(ino->dev, inode_block(ino->inum), false);
    memmove(b->cache, &ino->d, sizeof(DInode));
    bcache_write(b, false);
    bcache_release(b);
}


/* Return the blockno of the nth block of inode. Allocate blocks if necessary.
 * */
blockno inode_bmap(Inode *ino, unsigned nth) {
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
 *  @offest cursor offset. Indicates n bytes from start of the file.
 *  @sz     read size
 *  @return number of bytes read. Return -1 if failed.
 * */
int inode_read(Inode *ino, char *buf, unsigned offset, unsigned sz) {
    switch(ino->d.type) {
    case F_DEV:
        if (ino->d.major < 0 || ino->d.major > NDEV) return -1;
        if (!devices[ino->d.major].read)             return -1;
        return devices[ino->d.major].read(ino, buf, sz);
    case F_DIR:
    case F_FILE:
        if (ino->d.size < offset)         return -1;
        if ((unsigned)(-1) - offset < sz) return -1;
        if (offset + sz > ino->d.size) sz = ino->d.size - offset; // crops
        BNode *b;
        unsigned m;
        unsigned rd = 0; // bytes read
        while (rd < sz) {
            unsigned nth     = offset / BSIZE;
            blockno  blockno = inode_bmap(ino, nth);
            b                = bcache_read(ino->dev, blockno, false);
            m                = min(sz - rd, BSIZE - offset % BSIZE);
            memmove(buf, &b->cache[offset % BSIZE], m);
            rd     += m;
            offset += m;
            buf    += m;
            bcache_release(b);
        }
        return sz;
    }
}


/*! Write data to inode.
 *  @ino    Inode
 *  @buf    the buffer write from
 *  @offest cursor offset, indicates n bytes from start of the file.
 *  @sz     read size
 *  @return number of bytes written. Return -1 if failed.
 * */
int inode_write(Inode *ino, const char *buf, unsigned offset, unsigned sz) {
    switch (ino->d.type) {
    case F_DEV:
        if (ino->d.major < 0 || ino->d.major > NDEV) return -1;
        if (!devices[ino->d.major].write)            return -1;
        return devices[ino->d.major].write(ino, buf, sz);
    case F_DIR:
    case F_FILE:
        if (ino->d.size < offset)         return -1;
        if ((unsigned)(-1) - offset < sz) return -1;
        if (offset + sz > MAXFILE)        return -1;
        BNode *b;
        unsigned m;
        unsigned wt = 0;
        while (wt < sz) {
            unsigned nth     = offset / BSIZE;
            blockno  blockno = inode_bmap(ino, nth);
            b                = bcache_read(ino->dev, blockno, false);
            m                = min(sz - wt, BSIZE - offset % BSIZE);
            memmove(&b->cache[offset % BSIZE], buf, m);
            bcache_write(b, false);
            wt     += m;
            buf    += m;
            offset += m;
            bcache_release(b);
        }

        if (offset > ino->d.size) {
            ino->d.size = offset;
            inode_flush(ino);
        }

        return sz;
    }
}


/*! Get stat from inode */
void inode_stat(const Inode *ino, Stat *stat) {
    stat->dev   = ino->dev;
    stat->inum  = ino->inum;
    stat->nlink = ino->d.nlink;
    stat->size  = ino->d.size;
}
