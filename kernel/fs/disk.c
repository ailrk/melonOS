#include "err.h"
#include "fdefs.h"
#include "spinlock.h"
#include "driver/ide.h"
#include "driver/pic.h"
#include "trap/traps.h"
#include "fs/disk.h"

#define SECN (BSIZE/SECSZ)        // number of sectors per block
#define BLK2SEC(blk) (SECN * blk) // convert block number to sector number


/* The disk queue maintains a queue of pending BNodes waiting
 * for IDE interrupts. All bnodes are from `BCache`.
 * */
typedef struct DiskQueue {
    SpinLock lk;
    BNode *  head;
} DiskQueue;

DiskQueue disk_queue;


/*! Find the last node in disk queue */
static BNode *dq_last() {
    if (!disk_queue.head) return 0;
    BNode *last = disk_queue.head;
    while (last->qnext) {
        last = last->qnext;
    }
    return last;
}


/* Enqueue a new bnode task to the end of the queue.
 * */
static void dq_enqueue(BNode *b) {
    b->qnext = 0;
    if (!disk_queue.head) {
        disk_queue.head = b;
        return;
    }
    BNode *last = dq_last();
    last->qnext = b;
}


/* Dequeue the head fo the queue */
static BNode *dq_dequeue() {
    if (!disk_queue.head)
        return 0;

    BNode *b = disk_queue.head;
    disk_queue.head = disk_queue.head->qnext;
    return b;
}


/*! Send disk command for BNode b.
 *  If `b->dirty` is true, write `b->cache` to the disk, otherwise
 *  read the block b corresponds to into `b->cache`.
 * */
static void disk_cmd_request(BNode *b) {
    if (b->dirty) {
        ide_write_request(ATA_PRIMARY, ATA_SLAVE, b->cache, BLK2SEC (b->blockno), SECN);
    } else {
        ide_read_request(ATA_PRIMARY, ATA_SLAVE, BLK2SEC (b->blockno), SECN);
    }
}


/*! Read block */
static void read_block(BNode *b) {
    ide_read(ATA_PRIMARY, b->cache, SECN);
}


/*! Check if the cache synchronized synchronized with the disk */
static bool synced(BNode *b) {
    return b->valid && !b->dirty;
}


/*! Initialize disk */
void disk_init() {
    disk_queue.lk = new_lock("disk_queue.lk");
    if (!ide_has_secondary(ATA_PRIMARY)) {
        panic("Secondary disk doesn't exist");
    }
    pic_irq_unmask(I_IRQ_IDE);
}


/*! Syncronize the buffer cache with disk
 *  `disk_sync` will send disk command to IDE controller base on BNode flags.
 *  If `b->dirty`, write buffer to disk then clean `b->dirty`, set `b->valid`.
 *  If `!b->dirty` && `b->valid`, read from disk and set `b->valid`.
 *
 *  If `poll` is `false`, it works in asynchronous mode and will sleep immediately
 *  after sending the command. Once the disk is ready, it triggers an interrupt
 *  that calls `disk_handler` which reads the data and wake up the process.
 *
 *  If `poll` is true. `disk_sync` will poll until the device is ready.
 * */
void disk_sync(BNode *b, bool poll) {
    if (synced(b))
        panic("disc_sync: nothing to do");

    if (poll) {
        disk_cmd_request(b);
        ide_wait(ATA_PRIMARY);
        if (!b->valid) {
            read_block(b);
        }
        b->valid = true;
        b->dirty = false;

    } else {
        dq_enqueue(b);
        disk_cmd_request(b);
        while (!synced(b)); // TODO should sleep if block is not synced yet.
    }
}


/*! Handle disk interrupt.
 *  Pending tasks from a queue, the head of the queue
 *  is the current active request. The handler processes
 *  requests in the order until there's no more tasks left.
  * */
void disk_handler() {
    BNode *b = dq_dequeue();

    if (!b) return;

    if (!b->dirty) {
        read_block(b);
    }

    b->valid = true;
    b->dirty = false;

    if (disk_queue.head)
        disk_cmd_request(disk_queue.head);
}
