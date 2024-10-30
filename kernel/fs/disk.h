#pragma once
#include "fs/fdefs.h"


void disk_init ();
void disk_sync (BNode *b, bool poll);
void disk_handler ();
void disk_free (devno_t dev, blockno_t blockno);
