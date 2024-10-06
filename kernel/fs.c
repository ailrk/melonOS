#include "block.h"
#include "fs/file.h"
#include "fs/bcache.h"
#include "fs/inode.h"
#include "fs/disk.h"


void fs_init() {
    ftable_init();
    bcache_init();
    disk_init();
    block_super(0, 0, true);
    inode_init();
}
