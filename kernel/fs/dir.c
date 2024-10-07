#include "err.h"
#include "defs.h"
#include "string.h"
#include "fs/dir.h"


int dir_namecmp(const char *a, const char *b) {
    return strncmp(a, b, DIRNAMESZ);
}


Inode *dir_lookup(Inode *dir, char *name, unsigned *poff) {
    if (dir->d.type != F_DIR)
        panic("dir_lookup, not a dir");

}


Inode *dir_link(Inode *dir, char *name, inodenum inum) {

}
