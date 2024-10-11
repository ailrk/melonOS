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

        if (entry.inum == 0) continue;

        if (dir_namecmp(entry.name, name)  == 0) {
            if (offset) *offset = off;
            return inode_getc(dir->dev, entry.inum);
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


/* Index a path component of the path. If `n` > 0, get nth component from
 * the root to child. If `n` < 0, get -nth component from child to parent.
 * The index starts from 1.
 * e.g dir_pathidx("/abc/def/gdi", 1)  == "abc"
 *     dir_pathidx("/abc/def/gdi", 2)  == "def"
 *     dir_pathidx("/abc/def/gdi", -1) == "gdi"
 *
 * @path  a file path
 * @n     path component index
 * @part  output char pointer, needs to be at least DIRNAMESZ big.
 * */
bool dir_pathidx(char *path, signed n, char *part) {
    char  *savedpath;
    char  p[DIRNAMESZ];

    memmove(p, path, DIRNAMESZ);

    bool neg = false;
    if (n < 0) {
        neg = true;
        n   = -n;
        strrev(p);
    }

    int i = 1;
    for (char *tok  = strtok_r(p, "/", &savedpath);
               tok != 0;
               tok  = strtok_r(0, "/", &savedpath), ++i) {
        if (i == n) {
            memmove(part, tok, DIRNAMESZ);
            if (neg) {
                strrev(part);
            }
            return true;
        }
    }
    return false;
}


/*! Get an inode from a path name. If parent is true, return the direct parent directory
 *  of the path file.
 *  @path    absolute path
 *  @parent  if true, return the direct parent directory
 *  @return  if parent is false, return the inode to the path file, otherwise the direct
 *           parent
 *           of the path file.
 * */
Inode *dir_abspath(char *path, bool parent) {
    if (!path)          return 0;
    if (path[0] != '/') return 0; // not abs path

    Inode *ino = inode_getc(ROOTDEV, ROOTINO);
    char  *savedpath;
    inode_load(ino);

    for (char *tok  = strtok_r(path, "/", &savedpath);
               tok != 0;
               tok  = strtok_r(0, "/", &savedpath)) {
        offset_t off;
        Inode   *ino1;
        if ((ino1 = dir_lookup(ino, tok, &off)) == 0) return 0;
        switch (ino1->d.type) {
        case F_DIR:
            if (!strchr(savedpath, '/')) { // no more path component
                if (parent)
                    return ino;
                return ino1;
            }
            ino = ino1;
            continue;
        case F_FILE:
            if (strchr(savedpath, '/'))
                panic("dis_abspath: file is not the last path");
            if (parent)
                return ino;
            return ino1;
        default:
            panic("dis_abspath");
        }
    }

    return 0;
}
