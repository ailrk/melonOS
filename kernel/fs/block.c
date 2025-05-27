#include "defs.h"
#include "fdefs.fwd.h"
#include "string.h"
#include "err.h"
#include "fs/fdefs.h"
#include "fs/block.h"
#include "fs/bcache.h"

/* Block allocation
 *
 * On disk block structures:
 * [ super | inode .. | freemap .. | data .. ]
 *
 * */


#define BITS_PER_BLK (BSIZE * 8) // number of bmap bits per block
SuperBlock super_block;


void block_init(devno_t dev) {
    block_super(dev, &super_block, true);
}


/*! Zero a disk block */
void block_zero(devno_t dev, blockno_t blockno) {
    BNode *b = bcache_read(dev, blockno, false);
    memset(b->cache, 0, BSIZE);
    bcache_write(b, false);
    bcache_release(b);
}


/*! Read superblock. If `update` is true, read the superblock from the disk.
 *  @dev     Device number.
 *  @sb      Output. If it's 0, don't output anything.
 *  @update  If `update` is true, read the superblock from  the disk
 * */
void block_super(devno_t dev, SuperBlock *sb, bool update) {
    if (update) {
        BNode *b = bcache_read(dev, SUPERBLKNO, true);
        if (sb)
            memmove(sb, b->cache, sizeof(SuperBlock));
        if (sb != &super_block)
            memmove(&super_block, b->cache, sizeof(SuperBlock));
        bcache_release (b);
    } else {
        *sb = super_block;
    }
}


/*! Freemap bit address.
 *  The freemap stores blocks of bits to indicate whether a data block is in use.
 *  the nth bit from super_block.bmapstart indicates the availability of
 *  (super_block.datastart + nth bit).
 *
 *  To access the bit for a specific `blockno`, you need to  obtain the bitmap block with
 *  `super_block.bmapstart + (blockno - super_block.datastart)`, then access the nth
 *  bit in that block counts from MSB.
 *  */
typedef struct FreemapAddr {
    blockno_t bno;   // the block that contains the bit.
    unsigned  nbit; // n bits from `bmapstart`
} FreemapAddr;


/*! Get freemap bit address for `blockno`.
 * */
static FreemapAddr freemap_addr(unsigned blockno) {
    FreemapAddr addr;
    if (blockno < super_block.datastart) {
        panic("freemap_addr: blockno smaller than data start");
    }

    if (blockno > super_block.nblocks) {
        panic("freemap_addr: blockno larger then file system size");
    }

    unsigned off = blockno - super_block.datastart;
    addr.bno     = off / BITS_PER_BLK + super_block.bmapstart;
    addr.nbit    = off % BITS_PER_BLK;
    return addr;
}


/*! Check if `blockno` is used */
static bool freemap_check(devno_t dev, blockno_t blockno) {
    FreemapAddr addr = freemap_addr(blockno);
    BNode *b         = bcache_read(dev, addr.bno, false);
    char *freemap    = b->cache;
    bool ret         = freemap[addr.nbit / 8] & (0x80 >> (addr.nbit % 8));
    bcache_release(b);
    return ret;
}


/*! Set freemap bit */
static void freemap_set (devno_t dev, blockno_t blockno, bool used) {
    if (blockno < super_block.datastart) {
        panic ("freemap_set");
    }
    FreemapAddr    addr = freemap_addr(blockno);
    BNode         *b    = bcache_read(dev, addr.bno, false);
    unsigned char *byte = &((unsigned char *)b->cache)[addr.nbit / 8];
    if (used) {
        *byte |=  (0x80 >> (addr.nbit % 8));
    } else {
        *byte &= ~ (0x80 >> (addr.nbit % 8));
    }
    bcache_write(b, false);
    bcache_release(b);
}


/*! Search for the first free block in the freemap */
static unsigned freemap_search(devno_t dev, blockno_t *out) {
    unsigned nblks = super_block.datastart - super_block.bmapstart;
    for (unsigned off = 0; off < nblks; ++off) {
        BNode *b = bcache_read(dev, off + super_block.bmapstart, false);
        for (unsigned i = 0; i < sizeof (b->cache); ++i) {
            unsigned char byte = b->cache[i];

            if (byte == 0) {
                *out = super_block.datastart + off * BITS_PER_BLK;
                bcache_release(b);
                return true;
            }

            if (byte & 0xff) { // has 1
                unsigned n = 0;
                for (; byte & (1 << 7); byte <<= 1, ++n);
                blockno_t fbno = super_block.datastart + off * BITS_PER_BLK + n;
                *out = fbno;
                bcache_release (b);
                return true;
            }
        }
        bcache_release (b);
    }
    return false;
}


/*! Allocate a zeroed disk block
 *  This will look for the first free block from the freemap.
 *
 *  @return  the allocated blockno. 0 if the allocation is failed.
 * */
blockno_t block_alloc(devno_t dev) {
    blockno_t fbno;

    if (freemap_search(dev, &fbno)) {
        if (!fbno)
            panic("bad_alloc");
        freemap_set(dev, fbno, true);
        return fbno;
    }

    return 0;
}


/*! Free a block */
void block_free(devno_t dev, blockno_t blockno) {
    if (!freemap_check (dev, blockno)) {
        panic("block_free: block is already free");
        return;
    }
    freemap_set(dev, blockno, false);
}
