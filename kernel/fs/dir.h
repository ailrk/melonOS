#pragma once
#include <stddef.h>
#include "fdefs.fwd.h"
#include "fs/fdefs.h"


int    dir_namecmp(const char *a, const char *b);
Inode *dir_lookup(Inode *dir, char *name, offset_t *offset);
bool   dir_link(Inode *dir, DirEntry entry);
Inode *dir_abspath(char *path, size_t n);
