#pragma once

#include <stdbool.h>


#define ROOTINO 1  // root i-number


/* File types */
typedef enum FileType {
    FD_NONE,
    FD_PIPE,
    FD_INODE,
} FileType;


/* In disk representation of an inode */
typedef struct DInode {
    FileType        type;
    unsigned short  major; // major device number
    unsigned short  minor; // minor device number
    unsigned short  nlink; // number of links in fs
    unsigned int    size;
    unsigned int    addr;  // block address.
} DInode;


/* Memory representation of an inode */
typedef struct Inode {
    unsigned int dev;    // device number
    unsigned int ino;    // inode number
    int          nref;   // ref count
    bool         read;   // has been read from disk?
    DInode       dinode; // copy of disk inode.
} Inode;

