#pragma once
#include "fs/file.h"

/* Buffer cache node */
typedef struct BNode {
    struct BNode *next;
    struct BNode *prev;
    bool          dirty; // needs to be writtent to disk.
    bool          valid; // has been read from disk.
    unsigned int  nref;
    unsigned int  dev;
    unsigned int  blockno;
    char          cache[BSIZE];
} BNode;



void buffer_init();

BNode *buffer_get(unsigned int dev, unsigned int blockno);
BNode *buffer_read(unsigned int dev, unsigned int blockno);
BNode *buffer_release(BNode *b);
