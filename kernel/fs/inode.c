#include <stddef.h>
#include "mutex.h"
#include "stdlib.h"
#include "string.h"
#include "bcache.h"
#include "block.h"
#include "defs.h"
#include "err.h"
#include "inode.h"
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


/*! Get the block that the inode stored at. An inode is always completely stored within
 *  a block, you will not have an inode stored across 2 blocks.
 * */
inline static blockno_t get_inode_block(inodeno_t inum) {
    return inum / inode_per_block + super_block.inodestart;
}


void inode_init() {
    vga_printf("[\033[32mboot\033[0m] inode...");
    icache.lk = new_lock("icache.lk");
    vga_printf("\033[32mok\033[0m\n");
}


static bool dinode_read(Inode *ino, BNode *b) {
    if (b->blockno != get_inode_block(ino->inum)) return false;
    unsigned nth = ino->inum % inode_per_block;
    memmove(&ino->d, &b->cache[nth * sizeof(DInode)], sizeof(DInode));
    return true;
}


static bool dinode_write(Inode *ino, BNode *b) {
    if (b->blockno != get_inode_block(ino->inum)) return false;
    unsigned nth = ino->inum % inode_per_block;
    memmove(&b->cache[nth * sizeof(DInode)], &ino->d, sizeof(DInode));
    return true;
}


/*! Allocate an inode on disk and create an inode cache in icache. */
Inode *inode_allocate(devno_t dev, FileType type) {
    for (inodeno_t inum = ROOTINO + 1; inum < super_block.ninodes; ++inum) {
        BNode     *b    = bcache_read(dev, get_inode_block(inum), false);
        Inode     *ino  = inode_getc(dev, inum);
        if (!dinode_read(ino, b))
            return 0;

        if (ino->d.type == 0) {
            ino->dev    = dev;
            ino->d.type = type;
            if (!dinode_write(ino, b))
                return 0;
            bcache_write(b, false);
            bcache_release(b);
            return ino;
        }
        bcache_release(b);
    }
    panic("inode_allocate: no more space for inodes");
    __builtin_unreachable();
}


/*! Get an inode cache from icache. If the inode is not cached, allocate
 *  a inode in cache. This function does not read from the disk. To
 *  make sure it syncs with the disk, always call `inode_load` or
 *  `inode_lock` after it.
 *  Return 0 if there is not enough slots.
 * */
Inode *inode_getc(devno_t dev, inodeno_t inum) {
    for (unsigned i = 0; i < sizeof(icache.inodes); ++i) {
        Inode *ino = &icache.inodes[i];
        if (ino->nref >= 0 && ino->dev == dev && ino->inum == inum) {
            ino->nref++;
            return ino;
        }

        if (ino->nref == 0) { // allocate on free inode
            ino->nref = 1;
            ino->dev  = dev;
            ino->inum = inum;
            ino->read = 0;
            return ino;
        }
    }

    return 0;
}


/* Load an inode from disk. Return true if load succeeds */
bool inode_load(Inode *ino) {
    if (!ino)           return false;
    if (ino->nref == 0) return false;
    if (ino->read)      return true;

    blockno_t blockno = get_inode_block(ino->inum);
    BNode    *b       = bcache_read(ino->dev, blockno, false);

    dinode_read(ino, b);
    bcache_release(b);
    ino->read = 1;
    if (!ino->d.type)
        panic("inode_load: inode has no file type");
    return true;
}


/*! Flush in memory inode cache to disk. Needs to be called everytime inode field is updated. */
void inode_flush(Inode *ino) {
    BNode   *b   = bcache_read(ino->dev, get_inode_block(ino->inum), false);
    dinode_write(ino, b);
    bcache_write(b, false);
    bcache_release(b);
}


/*! Lock the inode. Load disk inode if necessary */
void inode_lock(Inode *ino) {
    if (!ino)           panic("inode_lock, invalid inode");
    if (ino->nref == 0) panic("inode_lock, inode is not used");
    lock_mutex(&ino->lk);
    if (!inode_load(ino)) unlock_mutex(&ino->lk);
}


/*! Unlock the locked inode. */
void inode_unlock(Inode *ino) {
    if (!ino)                     panic("inode_unlock: invalid inode");
    if (!holding_mutex(&ino->lk)) panic("inode_unlock: mutex not locked");
    if (ino->nref < 1)            panic("inode_unlock: inode is not being used");
    unlock_mutex(&ino->lk);
}


/*! Map direct blocks */
static blockno_t bmap0(Inode *ino, unsigned nth) {
    blockno_t blockno = 0;
    if ((blockno = ino->d.addrs[nth]) == 0) {
        blockno           = block_alloc(ino->dev);
        ino->d.addrs[nth] = blockno;
        inode_flush(ino);
    }
    return blockno;
}


/*! Map layer 1 indirect blocks */
static blockno_t bmap1(Inode *ino, unsigned nth) {
    blockno_t ptrsno  = 0;
    offset_t  offset  = nth - NDIRECT;
    blockno_t blockno = 0;

    if ((ptrsno = ino->d.addrs[nth]) == 0) {
        ptrsno            = block_alloc(ino->dev);
        ino->d.addrs[nth] = ptrsno;
        inode_flush(ino);
    }

    BNode *blockptrs = bcache_read(ino->dev, ptrsno, false);

    if ((blockno = ((unsigned *)blockptrs->cache)[offset]) == 0) {
        blockno = block_alloc(ino->dev);
        *(unsigned *)(&blockptrs->cache[offset]) = blockno;
        bcache_write(blockptrs, false);
    }

    bcache_release(blockptrs);
    return blockno;
}


/*! Return the blockno of the nth block of inode. Allocate blocks if necessary. */
blockno_t inode_bmap(Inode *ino, unsigned nth) {
    if (nth < NDIRECT) return bmap0(ino, nth);
    if (nth >= NDIRECT) return bmap1(ino, nth);
    return 0;
}


/*! Allocate blocks for `ino` until `offset` is allocated. Do nothing if the inode file size
 *  is already bigger than `offset`.
 * */
void inode_offmap(Inode *ino, offset_t offset) {
    unsigned n;

    if (ino->d.size > offset) return;

    for (n = 0; n <= offset / BSIZE; ++n) {
        if (!inode_bmap(ino, n))
            panic("inode_offmap");
    }

    if (offset > BSIZE && offset % BSIZE)
        if (!inode_bmap(ino, n))
            panic("inode_offmap");

    ino->d.size = offset;
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
int inode_read(Inode *ino, char *buf, offset_t offset, unsigned sz) {
    if (!ino->read) inode_load(ino);

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
            unsigned  nth     = offset / BSIZE;
            blockno_t blockno = inode_bmap(ino, nth);
            b                 = bcache_read(ino->dev, blockno, false);
            m                 = min(sz - rd, BSIZE - offset % BSIZE);
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
int inode_write(Inode *ino, const char *buf, offset_t offset, unsigned sz) {
    if (!ino->read) inode_load(ino);

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
            unsigned  nth     = offset / BSIZE;
            blockno_t blockno = inode_bmap(ino, nth);
            b                 = bcache_read(ino->dev, blockno, false);
            m                 = min(sz - wt, BSIZE - offset % BSIZE);

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
