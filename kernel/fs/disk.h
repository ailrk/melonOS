#pragma once
#include "driver/ide.h"
#include "fs/fdefs.h"


void disk_init();
void disk_sync(BNode *b);
void disk_handler();
void disk_free(DevNum dev, unsigned blockno);
