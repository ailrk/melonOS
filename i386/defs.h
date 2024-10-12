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
#define BSIZE       (SECSZ)        // block size
#define MAXBLKS     1000           // max file system size
#define NDEV        32             // max number of devices
#define NFILE       128            // max number of open files
#define NINODE      128            // max number of inodes
#define NOPBLKS     512            // max number of blocks writes
#define NBUF        (NOPBLKS * 5)  // max buffer size
#define NLOG        (NOPBLKS * 5)  // max log size
#define DIRNAMESZ   24             // directory name size
#define PATH_MAX    1024           // maximum path size
#define ROOTDEV     1              // device number of file system root
#define ROOTINO     0              // i-number for root directory


/* Inode pointer structures
 * Currently support 12 direct blocks and 1 singly indirect blocks.
 * */
#define NDIRECT     12                        // max number of direct blocks
#define NINDIRECT1  (BSIZE/sizeof(unsigned))  // max number of singly indirect blocks
#define NINOBLKS    (NDIRECT + 1)             // number of block address stored in inode addrs field
#define MAXFILEBLKS (NDIRECT + NINDIRECT1)    // max file block size
#define MAXFILE     (MAXFILEBLKS * BSIZE)     // max file size in bytes


/* Process parameters */
#define NPROC       64 // max number of processes
#define NOFILE      32 // max number of open files per process
