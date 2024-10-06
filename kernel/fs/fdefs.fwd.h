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
    FD_NONE,
    FD_PIPE,
    FD_INODE,
} FDType;


/* File type */
typedef enum FileType {
    F_DIR,  // directory
    F_FILE, // file
    F_DEV   // device
} FileType;



typedef struct File {
    FDType   type;
    int      nref;     // reference count
    bool     readable;
    bool     writable;
    unsigned offset;   // file cursor
    Inode   *ino;
} File;

