#pragma once

#include <stdbool.h>


/* Break cyclic dependency */

struct Inode;
typedef struct Inode Inode;


typedef unsigned blockno_t;
typedef unsigned inodeno_t;
typedef unsigned devno_t;
typedef unsigned offset_t; // byte offset

typedef struct Pipe Pipe;

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
    Inode   *ino;
    Pipe    *pipe;
    offset_t offset;   // file cursor
    FDType   type;
    int      nref;     // reference count
    bool     readable;
    bool     writable;
} File;
