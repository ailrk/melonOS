#pragma once
#include "fs/fdefs.h"


void   bcache_init();
BNode *bcache_read(devnum dev, blockno blockno, bool poll);
void   bcache_write(BNode *, bool poll);
BNode *bcache_release(BNode *b);
