#pragma once

#include "fs/fdefs.h"


void bcache_init();
BNode *bcache_read(unsigned dev, unsigned blockno);
BNode *bcache_release(BNode *b);
