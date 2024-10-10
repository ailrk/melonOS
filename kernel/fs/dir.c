#include "err.h"
#include "defs.h"
#include "inode.h"
#include "string.h"
#include "fdefs.fwd.h"
#include "fdefs.h"
#include "fs/dir.h"


int dir_namecmp(const char *a, const char *b) {
    return strncmp(a, b, DIRNAMESZ);
}


/*! Look up for a directory entry. If found, return the inode of the
 *  dir entry and set `offset` to offet of the entry.
 *  If not found, return 0;
 *  @dir     directory inode
 *  @name    directory name
 *  @offset  output pointer, stores the location of the entry in the director
 *  @return  inode corresponds to the directory entry.
 * */
Inode *dir_lookup(Inode *dir, char *name, offset_t *offset) {
    if (dir->d.type != F_DIR)
        panic("dir_lookup, not a dir");
    DirEntry entry;

    for (offset_t off = 0; off < dir->d.size; off += sizeof(DirEntry)) {
        if (inode_read(dir, (char *)&entry, off, sizeof(DirEntry)) != sizeof(DirEntry))
            panic("dir_lookup: invalid dir");

        if (entry.inum == 0)
            continue;

        if (dir_namecmp(entry.name, name)  == 0) {
            if (offset) *offset = off;
            return inode_get(dir->dev, entry.inum);
        }
    }

    return 0;
}


/*! Add a new directory entry to the directory */
bool dir_link(Inode *dir, DirEntry new_entry) {
    if (dir->d.type != F_DIR)
        panic("dir_link: not a dir");

    Inode *ino;
    DirEntry entry;

    if ((ino = dir_lookup(dir, new_entry.name, 0)) != 0) {  // exists
        inode_drop(ino);
        return false;
    }

    for (offset_t off = 0; off < dir->d.size; off += sizeof(DirEntry)) {
        if (inode_read(dir, (char *)&entry, off, sizeof(DirEntry)) != sizeof(DirEntry))
            panic("dir_link: invalid dir");

        if (entry.inum == 0) { // empty dir entry.
            if (inode_write(dir, (char *)&new_entry, off, sizeof(DirEntry) != sizeof(DirEntry)))
                panic("dir_link: write error");
            return true;
        }
    }

    return false; // no empty space
}


/*! Get an inode from a path name. */
Inode *dir_abspath(char *path, size_t n) {
    char buf[512];

    if (!path) return 0;
    if (path[0] != '/') return 0;

    char *tok = strtok(path, "/");

    Inode *root = inode_get(ROOTDEV, ROOTINO);
    Inode *ino;

    while (tok) {
        // read inode
        offset_t off;
        if ((ino = dir_lookup(root, tok, &off)) == 0) return 0;
        switch (ino->d.type) {
        case F_DIR:
            tok = strtok(0, "/");
            break;
        case F_FILE:
            break;
        default:
            panic("dis_abspath: impossible");
        }
    }

    return 0;
}
