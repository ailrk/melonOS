#include "debug.h"
#include "err.h"
#include "defs.h"
#include "inode.h"
#include "string.h"
#include "fdefs.fwd.h"
#include "fdefs.h"
#include "fs/dir.h"


/*! Initialize the file directory. The kernel will always hold
 *  a reference to the root directly.
 * */
void dir_init () {
    inode_getc (ROOTDEV, ROOTINO);
}


int dir_namecmp (const char *a, const char *b) {
    return strncmp (a, b, DIRNAMESZ);
}


/*! Look up for a directory entry. If found, return the inode of the
 *  dir entry and set `offset` to offet of the entry.
 *  If not found, return 0;
 *  @dir     directory inode
 *  @name    directory name
 *  @offset  output pointer, stores the location of the entry in the director
 *  @return  inode corresponds to the directory entry.
 * */
Inode *dir_lookup (Inode *dir, char *name, offset_t *offset) {
    if (dir->d.type != F_DIR)
        panic ("dir_lookup, not a dir");

    DirEntry entry;

    for (offset_t off = 0; off < dir->d.size; off += sizeof (DirEntry)) {
        if (inode_read (dir, (char *)&entry, off, sizeof (DirEntry)) != sizeof (DirEntry))
            panic ("dir_lookup: invalid dir");

        if (entry.inum == 0) continue;

        if (dir_namecmp (entry.name, name) == 0) {
            if (offset) *offset = off;
            return inode_getc (dir->dev, entry.inum);
        }
    }

    return 0;
}


/*! Add a new directory entry to the directory */
bool dir_link (Inode *dir, DirEntry new_entry) {
    if (dir->d.type != F_DIR)
        panic ("dir_link: not a dir");

    Inode   *ino;
    DirEntry entry;

    if ((ino = dir_lookup (dir, new_entry.name, 0)) != 0) {  // exists
        return false;
    }

    offset_t off;
    for (off = 0; off < dir->d.size; off += sizeof (DirEntry)) {
        if (inode_read (dir, (char *)&entry, off, sizeof (DirEntry)) != sizeof (DirEntry))
            panic ("dir_link: invalid dir");

        if (entry.inum == 0) { // empty dir entry.
            if (inode_write (dir, (char *)&new_entry, off, sizeof (DirEntry)) != sizeof (DirEntry))
                panic ("dir_link: write error");
            return true;
        }
    }

    // no enough space, allocate new blocks
    inode_offmap (dir, off + sizeof(DirEntry));
    if (inode_write (dir, (char *)&new_entry, off, sizeof (DirEntry)) != sizeof (DirEntry))
        panic ("dir_link: write error");

    return true;
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
bool dir_pathidx (const char *path, signed n, char *part) {
    char  *savedpath;
    char  p[DIRNAMESZ];

    memmove (p, path, DIRNAMESZ);

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
            strncpy (part, tok, DIRNAMESZ);
            if (neg) {
                strrev (part);
            }
            return true;
        }
    }
    return false;
}


/*! Get an inode from a path name. If parent is true, return the direct parent directory
 *  of the path file. `dir_abspath` always copy the path.
 *  `dir_abspath` does not increment the reference to an inode. It only look up for the inode
 *  and return a pointer it.
 *  @path    absolute path
 *  @parent  if true, return the direct parent directory
 *  @return  if parent is false, return the inode to the path file, otherwise the direct
 *           parent
 *           of the path file.
 * */
Inode *dir_abspath (const char *path, bool parent) {
    if (!path)          return 0;
    if (path[0] != '/') return 0; // not abs path

    char pathbuf[PATH_MAX];
    strncpy (pathbuf, path, PATH_MAX - 1);
    pathbuf[PATH_MAX - 1] = '\0';

    Inode *dir = inode_getc (ROOTDEV, ROOTINO);
    char  *savedpath;
    inode_load (dir);

    for (char *tok  = strtok_r (pathbuf, "/", &savedpath);
               tok != 0;
               tok  = strtok_r (0, "/", &savedpath)) {

        offset_t off;
        Inode   *ino;

        if (parent && !strchr(savedpath, '/')) {
            inode_drop (dir);
            return dir;
        }

        if ((ino = dir_lookup (dir, tok, &off)) == 0)  {
            inode_drop (dir);
            return 0;
        }

        inode_load(ino);

        switch (ino->d.type) {
        case F_DIR:
            if (!strchr (savedpath, '/')) { // no more path component
                inode_drop (ino);
                return ino;
            }
            inode_drop (dir);
            dir = ino;
        case F_DEV:
        case F_FILE:
            if (strchr (savedpath, '/'))
                panic ("dir_abspath: file is not the last path");
            inode_drop (ino);
            return ino;
        default:
            panic ("dir_abspath");
        }
    }

    return 0;
}
