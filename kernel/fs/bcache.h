#pragma once
#include "fs/fdefs.h"


void   bcache_init();
BNode *bcache_read(devnum dev, blockno blockno);
void   bcache_write(BNode *);
BNode *bcache_release(BNode *b);
