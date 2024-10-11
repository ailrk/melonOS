#include "block.h"
#include "defs.h"
#include "err.h"
#include "string.h"
#include "fs/fdefs.fwd.h"
#include "fs/fdefs.h"
#include "fs/file.h"
#include "fs/bcache.h"
#include "fs/inode.h"
#include "fs/disk.h"
#include "fs/dir.h"


void fs_init() {
    ftable_init();
    bcache_init();
    disk_init();
    block_super(0, 0, true);
    inode_init();
}


/*! Create a file */
Inode *fs_create(char *path, FileType type, uint16_t major, uint16_t minor) {
    Inode *dir;
    Inode *ino;
    char   name[DIRNAMESZ];

    // get parent dir
    if ((dir = dir_abspath(path, true)) == 0)
        return 0;

    // get path name
    if (!dir_pathidx(path, -1, name))
        return 0;

    devno_t dev = dir->dev;
    inode_load(dir);

    // file exists
    if ((ino = dir_lookup(dir, name, 0)) != 0) {
        if (ino->d.type == type)
            return ino;
        return 0;
    }

    // allocate
    if ((ino = inode_allocate(dev, type)) == 0)
        panic("fs_create: alloc");

    ino->d.major = major;
    ino->d.minor = minor;
    ino->d.nlink = 1;
    inode_flush(ino);

    DirEntry entry;
    if (ino->d.type == F_DIR) { // link .. and .
        dir->d.nlink++;
        inode_flush(ino);

        entry = (DirEntry){ .name = ".", .inum = ino->inum };
        if (!dir_link(ino, entry))
            panic("fs_create: link .");

        entry = (DirEntry){ .name = "..", .inum = dir->inum };
        if (!dir_link(ino, entry))
            panic("fs_create: link ..");
    }

    entry = (DirEntry){ .inum = ino->inum };
    memmove(entry.name, name, DIRNAMESZ);
    if (!dir_link(ino, entry)) {
        panic("fs_create: link ");
    }

    return ino;
}
