#pragma once
#include "fs/fdefs.h"


Inode *directory_lookup(Inode *ino, char *name, unsigned *poff);
