#pragma once
#include "fs/fdefs.h"


void   bcache_init();
BNode *bcache_read(devno_t dev, blockno_t blockno, bool poll);
void   bcache_write(BNode *, bool poll);
BNode *bcache_release(BNode *b);
