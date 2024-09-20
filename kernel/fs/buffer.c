#include "err.h"
#include "sleeplock.h"
#include "spinlock.h"
#include "fs/buffer.h"
#include "fs/fdefs.h"


/* BCache is a circular doubly linked list for buffering disk blocks in memory.
 * Caching blocks allow us to perform expensive updates in memory and
 * flush them to the disk when necessary.
 *
 * We need to ensure there is only one buffer cache node for one particular
 * disk block, otherwise it will causes consistency issues when multiple
 * buffers getting updated and overwrite each other.
 * */

typedef struct BCache {
    SpinLock lk;
    BNode *head;
    BNode buffer[NBUF];
} BCache;



BCache bcache;

/*! Init the doubly linked list for bcache. */
void bcache_init() {
    bcache.lk = new_lock("bcache.lk");
    BNode *head = &bcache.buffer[0];
    BNode *tail = &bcache.buffer[NBUF - 1];

    head->next = head;
    head->prev = tail;
    tail->next = head;
    tail->prev = tail;

    bcache.head = head;

    for (BNode *b = head + 1; b <= tail; ++b) {
        b->prev = bcache.head;
        b->prev->next = b;
        b->sleeplk = new_sleeplock("bnode.lk");
        bcache.head = b;
    }
}


/*! Lookup for block cached in bcache. If the block is not cached,
 *  return 0 */
static BNode* bcachce_lookup(unsigned dev, unsigned blockno) {
    BNode *b = bcache.head;

    do {
        if (b->dev == dev && b->blockno == blockno) {
            b->nref++;
            return b;
        }
        b = b->next;
    } while(b != bcache.head);

    return 0;
}

/*! Allocate an unused bcache node for the block. If no block is
 *  available return 0;
 * */
static BNode *bcache_allocate(unsigned dev, unsigned blockno) {
    BNode *b = bcache.head;

    do {
        if (b->nref == 0 && !b->dirty) {
            b->nref = 1;
            b->dev = dev;
            b->blockno = blockno;
            b->dirty = 0;
            b->valid = 0;
            return b;
        }
    } while (b != bcache.head);

    return 0;
}


/*! Look for buffer cache on dev. Allocate if the cache is not found. */
static BNode *bcache_get(unsigned dev, unsigned blockno) {
    BNode *b;
    lock(&bcache.lk);

    if ((b = bcachce_lookup(dev, blockno))) {
        unlock(&bcache.lk);
        lock_sleep(&b->sleeplk);
        return b;
    }

    if ((b = bcache_allocate(dev, blockno))) {
        unlock(&bcache.lk);
        lock_sleep(&b->sleeplk);
        return b;
    }

    return 0;
}
