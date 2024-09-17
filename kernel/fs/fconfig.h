#pragma once

/* Configurartion for the file system parameters */

#define BSIZE   512            // block size
#define NDEV    32             // max number of devices
#define NFILE   128            // max number of open filese in the system
#define NOPBLKS 10             // max # of blocks any FS op writes
#define NBUF    (NOPBLKS * 5)  // max buffer size
#define NLOG    (NOPBLKS * 5)  // max log size
#define DIR_SZ  512            // max number of directories
#define MAXBLKS 1000           // max file system size
