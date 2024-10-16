#pragma once
#include <stdint.h>
#include "fs/fdefs.h"


void   fs_init();
Inode *fs_create(const char *path, FileType type, uint16_t major, uint16_t minor);
int    fs_fdalloc(File *f);
