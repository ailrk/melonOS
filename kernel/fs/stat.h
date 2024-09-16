#pragma once

#include <stdint.h>

#define T_DIR  1 // directory
#define T_FILE 2 // file
#define T_DEV  3 // device

typedef struct Stat {
    short        type;  // file type
    int          dev;   // disk device
    int          ino;   // inode number
    short        nlink; // number of links
    unsigned int size;  // file size
} Stat;
