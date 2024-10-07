#pragma once
#include <stdbool.h>
#include "fdefs.fwd.h"
#include "fdefs.h"

#define ROOTINO 1  // root i-number


void    inode_init();
Inode  *inode_get(devnum dev, inodenum inum);
Inode  *inode_allocate(devnum dev, FileType type);
void    inode_flush(Inode *ino);
blockno inode_bmap(Inode *ino, unsigned nth);
Inode  *inode_dup(Inode *ino);
bool    inode_load(Inode *ino);
void    inode_lock(Inode *ino);
void    inode_unlock(Inode *ino);
void    inode_drop(Inode *ino);
int     inode_read(Inode *ino, char *buf, unsigned offset, unsigned sz);
int     inode_write(Inode *ino, const char *buf, unsigned offset, unsigned sz);
void    inode_stat(const Inode *ino, Stat *stat);
