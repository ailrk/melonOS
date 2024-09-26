#include "string.h"
#include "err.h"
#include "fs/fdefs.h"
#include "fs/block.h"
#include "fs/bcache.h"

/* Block allocation
 *
 * On disk block structures:
 * [ boot | super | log | inode .. | freemap .. | data .. ]
 * */


#define SUPERBLKNO   20
#define BITS_PER_BLK (BSIZE * 8) // number of bmap bits per block


SuperBlock super_block;


void block_init(DevNum dev) {
    block_super(dev, &super_block, true);
}


/*! Zero a disk block */
void block_zero(DevNum dev, unsigned blockno) {
    BNode *b = bcache_read(dev, blockno);
    memset(b->cache, 0, BSIZE);
    bcache_write(b);
    bcache_release(b);
}


/*! Read superblock. If `update` is true, read the superblock
 *  from the disk.
 * */
void block_super(DevNum dev, SuperBlock *sb, bool update) {
    if (update) {
        BNode *b = bcache_read(dev, SUPERBLKNO);
        memmove(&sb, b->cache, BSIZE);
        bcache_release(b);
    } else {
        *sb = super_block;
    }
}


typedef enum FreeMapOp {
    O_CHECK,
    O_SET,
    O_UNSET,
    O_SEARCH
} FreeMapOp;


typedef enum FreeMapRet {
    F_0,
    F_1,
    F_DONE,
    F_FULL,
    F_ERR
} FreeMapRet;


/*! Operating on freemap blocks.
 *  It assumes `super_block` is up to date.
 *  @dev     device number
 *  @op      free map operation
 *  @blockno the blockno to operate on
 *  @out     out ptr
 *  @outsz   size of data by outptr
 * */
static FreeMapRet freemap_ctl(DevNum dev, FreeMapOp op, unsigned blockno, void *out, size_t outsz) {
    FreeMapRet ret = F_ERR;

    switch (op) { // type check
        case O_CHECK:
            if (outsz != sizeof(unsigned))
                panic("freemap_ctl: O_SEARCH");
        default:
            if (out != 0 || outsz != 0)
                panic("freemap_ctl: unused out");
    }

    if (op >= O_CHECK && op <= O_UNSET) {
        unsigned pg    = blockno / BITS_PER_BLK;
        unsigned nbit  = blockno - (pg * BITS_PER_BLK);
        unsigned bbno  = super_block.bmapstart + pg;
        BNode *b       = bcache_read(dev, bbno);
        char *freemap  = b->cache;
        switch (op) {
            case O_CHECK:
                if (freemap[nbit / 8] & (1 << (nbit % 8))) {
                    ret = F_1;
                } else {
                    ret = F_0;
                }
                break;
            case O_SET:
                freemap[nbit / 8] |= (1 << (nbit % 8));
                ret = F_DONE;
                break;
            case O_UNSET:
                freemap[nbit / 8] &= ~(1 << (nbit % 8));
                ret = F_DONE;
                break;
            default:
                panic("freemap_ctl");
        }
        bcache_release(b);
        return ret;
    }

    if (op == O_SEARCH) {
        unsigned fmapblks = super_block.datastart - super_block.bmapstart;
        for (unsigned pg = 0; pg < fmapblks; ++pg) {
            BNode *b      = bcache_read(dev, super_block.bmapstart + pg);
            char *freemap = b->cache;
            for (unsigned i = 0; i < sizeof(freemap); ++i) {
                if (freemap[i] & 0xff) {
                    unsigned n       = __builtin_ffs(freemap[i]);
                    unsigned fbno    = pg * BITS_PER_BLK + n;
                    *(unsigned *)out = fbno;
                    return F_DONE;
                }
            }
        }
        return F_FULL;
    }
    return ret;
}


/*! Allocate a zeroed disk block
 *  This will look for the first free block from the freemap.
 *
 *  @return  the allocated blockno. 0 if the allocation is failed.
 * */
unsigned block_alloc(DevNum dev) {
    unsigned blockno = super_block.datastart;
    unsigned fbno;

    switch (freemap_ctl(dev, blockno, O_SEARCH, &fbno, sizeof(unsigned))) {
        case F_DONE:
            if (!fbno)
                panic("bloc_alloc");

            if (freemap_ctl(dev, fbno, O_SET, 0, 0) != F_DONE)
                panic("bloc_alloc");

            return fbno;
            break;
        case F_FULL:
            perror("block_alloc: disk full");
            return 0;
        default:
            panic("block_alloc");
            __builtin_unreachable();
    }
}


/*! Free a block */
void block_free(DevNum dev, unsigned blockno) {
    switch(freemap_ctl(dev, blockno, O_CHECK, 0, 0)) {
        case F_0:
            panic("block_free: block is already free");
            break;
        case F_1:
            if (freemap_ctl(dev, blockno, O_UNSET, 0, 0) != F_DONE)
                panic("block_free: can't free block");
            break;
        default:
            panic("block_free");
            break;
    }
}
