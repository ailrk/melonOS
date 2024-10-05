#pragma once
#include <stdbool.h>
#include "fs/fdefs.h"


void  ftable_init();
File *allocate_file();
File *dup_file(File *);
void  close_file(File *);
void  stat_file(File *, Stat *);
void  read_file(File *, char *, int);
void  write_file(File *, char *, int);
