#pragma once

#define MAJOR_VER "0"
#define MINOR_VER "0"
#define PATCH_VER "1"
#define VERSION MAJOR_VER "." MINOR_VER "." PATCH_VER


/* Kernel stack size */
#define KSTACK_SZ   4096


/* Disk */
#define SECSZ       512
#define BOOTLDSECN  20


/* File system parameters */
#define SUPERBLKNO  0
#define BSIZE       (SECSZ)                   // block size
#define MAXBLKS     1000                      // max file system size
#define NDEV        32                        // max number of devices
#define NFILE       128                       // max number of open files
#define NINODE      128                       // max number of inodes
#define NOPBLKS     512                       // max # of blocks writes
#define NBUF        (NOPBLKS * 5)             // max buffer size
#define NLOG        (NOPBLKS * 5)             // max log size
#define DIRNAMESZ   128                       //  directory name size


/* Inode pointer structures
 * Currently support 12 direct blocks and 1 singly indirect blocks.
 * */
#define NDIRECT     12                        // max # of direct blocks
#define NINDIRECT1  (BSIZE/sizeof(unsigned))  // max # of singly indirect blocks
#define NINOBLKS    (NDIRECT + 1)             // Inode addresses


/* Max file block size */
#define MAXFILE     (NDIRECT + NINDIRECT1)


/* Process parameters */
#define NPROC       64 // max number of processes
#define NOFILE      32 // max number of open files per process
