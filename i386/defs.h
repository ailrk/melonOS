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
#define BSIZE       (SECSZ)                  // block size
#define NDEV        32                       // max number of devices
#define NFILE       128                      // max number of open files
#define NINODE      128                      // max number of inodes
#define NOPBLKS     512                      // max # of blocks writes
#define NBUF        (NOPBLKS * 5)            // max buffer size
#define NLOG        (NOPBLKS * 5)            // max log size
#define NDIRECT     12                       // max number of direct blocks
#define NINDIRECT   (BSIZE/sizeof(unsigned)) // max number of indirect blocks
#define MAXFILE     (NDIRECT + NINDIRECT)    // max number of files
#define DIRNAMESZ   128                      //  directory name size
#define MAXBLKS     1000                     // max file system size


/* Process parameters */
#define NPROC       64 // max number of processes
#define NOFILE      32 // max number of open files per process
