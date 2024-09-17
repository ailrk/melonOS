#include "fs.h"

/* The file system on disk structure 
 * | boot | superblock | log  | inodes | bitmaps | data blocks  ... |
 *    0         1         2..
 * */
