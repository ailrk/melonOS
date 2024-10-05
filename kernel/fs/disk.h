#pragma once
#include "fs/fdefs.h"


void disk_init();
void disk_sync(BNode *b);
void disk_handler();
void disk_free(devnum dev, blockno blockno);
