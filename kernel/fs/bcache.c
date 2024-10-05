#include "err.h"
#include "process/mutex.h"
#include "process/spinlock.h"
#include "fs/bcache.h"
#include "fs/disk.h"
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
    BNode *  head;
    BNode    buffer[NBUF];
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
        b->mutex = new_mutex("bnode.mtx");
        bcache.head = b;
    }
}


/*! Lookup for block cached in bcache. If the block is not cached,
 *  return 0 */
static BNode* bcachce_lookup(unsigned dev, blockno blockno) {
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
static BNode *bcache_allocate(unsigned dev, blockno blockno) {
    BNode *b = bcache.head;

    do {
        if (b->nref == 0 && !b->dirty) {
            b->nref    = 1;
            b->dev     = dev;
            b->blockno = blockno;
            b->dirty   = 0;
            b->valid   = 0;
            return b;
        }
        b = b->next;
    } while (b != bcache.head);

    return 0;
}


/*! Look for buffer cache on dev. Allocate if the cache is not found. */
static BNode *bcache_acquire(unsigned dev, blockno blockno) {
    BNode *b;

    if ((b = bcachce_lookup(dev, blockno))) {
        return b;
    }

    if ((b = bcache_allocate(dev, blockno))) {
        return b;
    }

    return 0;
}


/*! Read a `BNode` from blockno.
 *  `bcache_read` allocates a node from `bcache` and locks the
 *  mutex on that node. The node needs to be released manually
 *  to be available in the bcache again.
  * */
BNode *bcache_read(devnum dev, blockno blockno) {
    BNode *b;
    if ((b = bcache_acquire(dev, blockno)) == 0) {
        panic("bcache read");
    }

    if (!b->valid) {
        disk_sync(b);
    }
    return b;
}


/*! Write `BNode` to blockno */
void bcache_write(BNode *b) {
    b->dirty = true;
    disk_sync(b);
}


/*! Clean up the node and move it to the head of the cache.
 * */
static void bcache_free(BNode *b) {
    b->nref--;
    if (b->nref == 0) {
        b->next->prev     = b->prev;
        b->prev->next     = b->next;
        b->next           = bcache.head;
        b->prev           = bcache.head->prev;
        bcache.head->prev = b;
        bcache.head       = b;
    }
}


/*! Release the BNode.
 * */
BNode *bcache_release(BNode *b) {
    bcache_free(b);
}
