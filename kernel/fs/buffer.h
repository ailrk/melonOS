#pragma once

#include "fs/fdefs.h"


void buffer_init();
BNode *buffer_get(unsigned int dev, unsigned int blockno);
BNode *buffer_read(unsigned int dev, unsigned int blockno);
BNode *buffer_release(BNode *b);
