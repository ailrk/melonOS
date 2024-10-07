#pragma once
#include "fdefs.fwd.h"
#include "fs/fdefs.h"


int    dir_namecmp(const char *a, const char *b);
Inode *dir_lookup(Inode *dir, char *name, unsigned *poff);
Inode *dir_link(Inode *dir, char *name, inodenum inum);
