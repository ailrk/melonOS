#pragma once

#include "stat.h"
#include <stdbool.h>

/* File descriptors */

#define NDEV  32  // max number of devices
#define NFILE 128 // max number of open filese in the system


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
    bool         writable;
    unsigned int offset;
    Inode *ip;
} File;


typedef struct Dev {
    int (*read)(Inode *ino, char * buf, int n);
    int (*write)(Inode *ino, char * buf, int n);
} Dev;

extern Dev devs[NDEV];


#define DEV_VGA      1
#define DEV_PS2KBD   2
#define DEV_PS2MOUSE 3


void file_init();
File *allocate_file();
File *dup_file(File *);
void close_file(File *);
void stat_file(File *, Stat *);
void read_file(File *, char *, int);
void write_file(File *, char *, int);
