#pragma once

#include <stdbool.h>

/* File descriptors */


/* Memory representation of an inode */
typedef struct Inode {
    unsigned int dev;  // device number
    unsigned int ino;  // inode number
    int          nref; // ref count
} Inode;


/* File types */
typedef enum FileType {
    FD_NONE,
    FD_PIPE,
    FD_INODE,
} FileType;


typedef struct File {
    FileType     type;
    int          nref;       // reference count
    bool         readable;
    bool         writale;
    unsigned int offset;
    Inode *ip;
} File;
