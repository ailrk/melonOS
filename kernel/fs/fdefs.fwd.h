#pragma once

#include <stdbool.h>


/* Break cyclic dependency */

struct Inode;
typedef struct Inode Inode;


typedef unsigned InodeNum;
typedef unsigned DevNum;


/* File types */
typedef enum FileType {
    FD_NONE,
    FD_PIPE,
    FD_INODE,
} FileType;


typedef struct File {
    FileType type;
    int      nref;       // reference count
    bool     readable;
    bool     writable;
    unsigned offset;
    Inode *  ino;
} File;
