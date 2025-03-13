#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "defs.h"
#include "fdefs.fwd.h"
#include "process/mutex.h"


typedef struct SuperBlock {
    unsigned  nblocks;    // total size of fs in blocks
    unsigned  ninodes;    // number of inodes
    unsigned  ndata;      // number of data blocks
    blockno_t inodestart; // blockno of the first ino
    blockno_t bmapstart;  // blockno of the first free bit map
    blockno_t datastart;  // blockno of the first ino
} SuperBlock;


/* In disk representation of an inode
 * `addrs` holds inode block addresses. The first 12 addresses are
 *  direct blocks, 13th address is singly indirect address, 14th
 *  is doubly indirect address, 15th is triply indirect address.
 *  Small files only needs to acess direct blocks, as file size grows,
 *  more indirections are added.
 * */
typedef struct DInode {
    FileType        type;
    unsigned short  major; // major device number
    unsigned short  minor; // minor device number
    unsigned short  nlink; // number of links in fs
    unsigned        size;  // size of the file
    blockno_t       addrs[NINOBLKS];  // block address.
} __attribute__((packed)) DInode;


/* Memory representation of an inode */
typedef struct Inode {
    devno_t   dev;  // device number
    inodeno_t inum; // The index of inode from `super_block.inodestart`
    int       nref; // ref count
    Mutex     lk;
    bool      read; // has been read from disk?
    DInode    d;    // copy of disk inode.
} Inode;


/* Number of inodes per block */
static const size_t inode_per_block = BSIZE / sizeof (DInode);


/*! Get the block that the inode stored at. An inode is always completely stored within
 *  a block, you will not have an inode stored across 2 blocks.
 * */
inline static blockno_t get_inode_block(inodeno_t inum, SuperBlock *sb) {
    return inum / inode_per_block + sb->inodestart;
}


/* Buffer cache node */
typedef struct BNode {
    struct BNode *next;
    struct BNode *prev;
    struct BNode *qnext; // next node on disk queue.
    Mutex         mutex;
    bool          dirty; // needs to be writtent to disk.
    bool          valid; // has been read from disk.
    unsigned      nref;
    devno_t       dev;
    blockno_t     blockno;
    char          cache[BSIZE];
} BNode;


/* Devices need to implement this interface */
typedef struct Dev {
    int (*read) (Inode *ino, char *addr, int n);
    int (*write) (Inode *ino, char *addr, int n);
} Dev;


/* Directory entry */
typedef struct DirEntry {
  inodeno_t inum;
  char      name[DIRNAMESZ];
} DirEntry;


/* file status */
#define T_DIR  1 // directory
#define T_FILE 2 // file
#define T_DEV  3 // device


typedef struct Stat {
    short      type;  // file type
    devno_t    dev;   // disk device
    inodeno_t  inum;  // inode number
    short      nlink; // number of links
    unsigned   size;  // file size
} Stat;
