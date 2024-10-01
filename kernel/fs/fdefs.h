#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "defs.h"
#include "fdefs.fwd.h"
#include "mutex.h"


typedef struct SuperBlock {
    unsigned nblocks;    // total size of fs in blocks
    unsigned ninodes;    // number of inodes
    unsigned ndata;      // number of data blocks
    unsigned inodestart; // blockno of the first ino
    unsigned bmapstart;  // blockno of the first free bit map
    unsigned datastart;  // blockno of the first ino
} SuperBlock;


/* In disk representation of an inode */
typedef struct DInode {
    FileType        type;
    unsigned short  major; // major device number
    unsigned short  minor; // minor device number
    unsigned short  nlink; // number of links in fs
    unsigned        size;
    unsigned        addr;  // block address.
} DInode;


/* Memory representation of an inode */
typedef struct Inode {
    DevNum   dev;    // device number
    InodeNum inum;   // inode number
    int      nref;   // ref count
    bool     read;   // has been read from disk?
    DInode   dinode; // copy of disk inode.
} Inode;


/* Buffer cache node */
typedef struct BNode {
    struct BNode * next;
    struct BNode * prev;
    struct BNode * qnext; // next node on disk queue.
    Mutex          mutex;
    bool           dirty; // needs to be writtent to disk.
    bool           valid; // has been read from disk.
    unsigned       nref;
    DevNum         dev;
    unsigned       blockno;
    char           cache[BSIZE];
} BNode;


/* Devices need to implement this interface */
typedef struct Dev {
    int (*read)(Inode *ino, char * addr, int n);
    int (*write)(Inode *ino, char * addr, int n);
} Dev;


/* Directory entry */
typedef struct DirEntry {
  InodeNum  inum;
  char      name[DIR_SZ];
} DirEntry;


/* file status */
#define T_DIR  1 // directory
#define T_FILE 2 // file
#define T_DEV  3 // device

typedef struct Stat {
    short    type;  // file type
    DevNum   dev;   // disk device
    InodeNum inum;  // inode number
    short    nlink; // number of links
    unsigned size;  // file size
} Stat;
