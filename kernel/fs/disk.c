#include "spinlock.h"
#include "mutex.h"
#include "string.h"
#include "drivers/ide.h"
#include "fs/disk.h"
#include "fs/buffer.h"

#define SECN (BSIZE/SECTSZ)
#define BLK2SEC(blk) (SECN * blk)


typedef struct DiskQueue {
    SpinLock lk;
    BNode *head;
} DiskQueue;

DiskQueue disk_queue;


/*! Initialize disk */
void disk_init() {
    disk_queue.lk = new_lock("disk_queue.lk");
}


/*! Write a block */
static void write_block(BNode *b) {
    ide_write_sync(ATA_PRIMARY, b->cache, BLK2SEC(b->blockno), SECN);
}


/*! Send a read block request. The block can be read asynchronously */
static void read_block_request(BNode *b) {
    ide_read_request(ATA_PRIMARY, BLK2SEC(b->blockno), SECN);
}


/*! Read block */
static void read_block(BNode *b) {
    ide_read(ATA_PRIMARY, b->cache, SECN);
}


/*! Check if the cache synchronized synchronized with the disk */
static bool synced(BNode *b) {
    return b->valid && !b->dirty;
}


/*! Zero a disk block */
void disk_zero(DevNum dev, unsigned blockno) {
    BNode *b = bcache_read(dev, blockno);
    memset(b->cache, 0, BSIZE);
    bcache_write(b);
    bcache_release(b);
}


/*! Syncronize the buffer cache with disk
 *  If `b->dirty`, write buffer to disk then clean `b->dirty`, set `b->valid`.
 *  If `!b->dirty` && `b->valid`, read from disk and set `b->valid`.
 * */
void disk_sync(BNode *b) {
    if (synced(b))
        return;

    b->qnext = 0;

    BNode **p;
    for(p = &disk_queue.head; *p; p = &(*p)->qnext);
    *p =  b;

    if (b->dirty) {
        write_block(b);
    } else if (b->valid) {
        read_block_request(b);
    }
}


/*! Handle disk interrupt */
void disk_handler() {
    BNode *b;

    if((b = disk_queue.head) == 0) {
        return;
    }

    disk_queue.head = b->next;

    if (!b->dirty) {
        read_block(b);
    }

    b->valid = true;
    b->dirty = false;
}
