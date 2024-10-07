#include "err.h"
#include "defs.h"
#include "string.h"
#include "fdefs.h"
#include "fs/dir.h"


int dir_namecmp(const char *a, const char *b) {
    return strncmp(a, b, DIRNAMESZ);
}


/*! Look up for a dir entry. If found, set `offset` to offet of the entry.
 * */
Inode *dir_lookup(Inode *dir, char *name, offset_t *offset) {
    if (dir->d.type != F_DIR)
        panic("dir_lookup, not a dir");
    DirEntry entry;
}


/*! Add a new directory entry to the directory */
Inode *dir_link(Inode *dir, DirEntry entry) {
}
