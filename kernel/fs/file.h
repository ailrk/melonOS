#pragma once

#include <stdbool.h>
#include "fs/fdefs.h"

/* File descriptors */

extern Dev devs[NDEV];


#define DEV_VGA      1
#define DEV_PS2KBD   2
#define DEV_PS2MOUSE 3


void ftable_init();
File *allocate_file();
File *dup_file(File *);
void close_file(File *);
void stat_file(File *, Stat *);
void read_file(File *, char *, int);
void write_file(File *, char *, int);
