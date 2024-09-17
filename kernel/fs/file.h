#pragma once

#include <stdbool.h>
#include "fs/stat.h"
#include "fs/fconfig.h"
#include "fs/inode.h"

/* File descriptors */

typedef struct File {
    FileType     type;
    int          nref;       // reference count
    bool         readable;
    bool         writable;
    unsigned int offset;
    Inode *ip;
} File;


/* Devices need to implement this interface */
typedef struct Dev {
    int (*read)(Inode *ino, char * addr, int n);
    int (*write)(Inode *ino, char * addr, int n);
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
