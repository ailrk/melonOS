#include "fs/disk.h"
#include "spinlock.h"
#include "mutex.h"

#define SECSPBLOCK (BSIZE / SECTSZ)

typedef struct DiskQueue {
    SpinLock lk;
    BNode *head;
} DQueue;

DiskQueue disk_queue;


/*! Initialize disk */
void disk_init() {
    disk_queue.lk = new_lock();
}


static void write_block(void *src, unsigned lba) {
    ide_write_secn(ATA_PRIMARY, src, lba, SECSPBLOCK);
}


static void read_block(void *dst, unsigned lba) {
    ide_read_secn(ATA_PRIMARY, dst, lba, SECSPBLOCK);
}


static bool synced(BNode *b) {
    return b->valid && !b->dirty;
}


/*! Syncronize the buffer cache with disk
 *  If `b->dirty`, write buffer to disk then clean `b->dirty`, set `b->valid`.
 *  If `!b->dirty` && `b->valid`, read from disk and set `b->valid`.
 * */
void disk_sync(BNode *b) {
    if (!holding_mutex(b->mutex))
        panic("not holding mutex");
    if (synced())
        return;

    lock(&disk_queue->lk);
    b->qnext = 0;

    BNode **p;
    for(p = &disk_queue.head; p; p = &(*p)->qnext);
    *p =  b;

    if (b->dirty) {
        write_block(b->cache, b->blockno * SECSPBLOCK);
    } else (b->valid) {
        read_block(b->cache, b->blockno * SECSPBLOCK);
    }

    while (!synced(b))
        sleep(b, &disk_queue->lk);

    unlock(&disk_queue->lk);
}


/*! Handle disk interrupt */
void disk_handler() {

}
