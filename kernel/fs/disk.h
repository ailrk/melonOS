#pragma once

#include "drivers/ide.h"
#include "fs/fdefs.h"


void disk_init();
void disk_sync(BNode *b);
void disk_handler();
