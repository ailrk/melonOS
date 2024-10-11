#pragma once
#include <stdbool.h>
#include "fdefs.fwd.h"
#include "fdefs.h"

#define ROOTINO 0  // i-number for root directory


void      inode_init();
Inode    *inode_getc(devno_t dev, inodeno_t inum);
Inode    *inode_allocate(devno_t dev, FileType type);
void      inode_flush(Inode *ino);
blockno_t inode_bmap(Inode *ino, unsigned nth);
Inode    *inode_dup(Inode *ino);
bool      inode_load(Inode *ino);
void      inode_lock(Inode *ino);
void      inode_unlock(Inode *ino);
void      inode_drop(Inode *ino);
int       inode_read(Inode *ino, char *buf, offset_t offset, unsigned sz);
int       inode_write(Inode *ino, const char *buf, offset_t offset, unsigned sz);
void      inode_stat(const Inode *ino, Stat *stat);
