#pragma once
#include <stdbool.h>
#include "fdefs.fwd.h"
#include "fdefs.h"

#define ROOTINO 1  // root i-number


void   inode_init(DevNum dev);
Inode *inode_get(DevNum dev, InodeNum inum);
Inode *inode_allocate(DevNum dev, FileType type);
void   inode_update(Inode *ino);
void   inode_dup(Inode *ino);
void   inode_lock(Inode *ino);
void   inode_unlock(Inode *ino);
void   inode_put(Inode *ino);
int    inode_read(Inode *ino, char *tgt, unsigned off, unsigned n);
int    inode_write(Inode *ino, char *tgt, unsigned off, unsigned n);
void   inode_stat(const Inode *ino, Stat *stat);
