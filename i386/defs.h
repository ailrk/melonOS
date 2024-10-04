#pragma once

#define MAJOR_VER "0"
#define MINOR_VER "0"
#define PATCH_VER "1"
#define VERSION MAJOR_VER "." MINOR_VER "." PATCH_VER

#define KSTACK_SZ   4096

#define SECSZ       512
#define BOOTLDSECN  20

/* File system parameters */
#define SUPERBLKNO  0
#define BSIZE       (SECSZ)        // block size
#define NDEV        32             // max number of devices
#define NFILE       128            // max number of open filese in the system
#define NINODE      128            // max number of inodes
#define NOPBLKS     512            // max # of blocks any FS op writes
#define NBUF        (NOPBLKS * 5)  // max buffer size
#define NLOG        (NOPBLKS * 5)  // max log size
#define DIR_SZ      512            // max number of directories
#define MAXBLKS     1000           // max file system size


/* Process parameters */
#define NPROC       64 // max number of processes
#define NOFILE      32 // max number of open files per process
