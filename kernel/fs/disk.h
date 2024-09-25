#pragma once

#include "drivers/ide.h"
#include "fs/fdefs.h"


void disk_init();
void disk_sync(BNode *b);
void disk_handler();
void disk_zero(DevNum dev, unsigned blockno);
void disk_alloc(DevNum dev);
void disk_free(DevNum dev, unsigned blockno);
