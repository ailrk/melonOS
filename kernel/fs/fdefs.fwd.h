#pragma once

#include <stdbool.h>


/* Break cyclic dependency */

struct Inode;
typedef struct Inode Inode;


typedef unsigned blockno;
typedef unsigned inodenum;
typedef unsigned devnum;


/* File descriptor types */
typedef enum FDType {
    FD_NONE  = 0,
    FD_PIPE  = 1,
    FD_INODE = 2,
} FDType;


/* File type */
typedef enum FileType {
    F_DIR  = 1, // directory
    F_FILE = 2, // file
    F_DEV  = 3  // device
} FileType;



typedef struct File {
    FDType   type;
    int      nref;     // reference count
    bool     readable;
    bool     writable;
    unsigned offset;   // file cursor
    Inode   *ino;
} File;
