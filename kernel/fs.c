#include "fs/file.h"
#include "fs/bcache.h"
#include "fs/inode.h"
#include "fs/disk.h"


void fs_init() {
    ftable_init();
    bcache_init();
    disk_init();
    inode_init();
}
