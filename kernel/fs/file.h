#pragma once
#include <stdbool.h>
#include "fs/fdefs.h"


void  ftable_init ();
File *file_allocate ();
File *file_dup (File *);
void  file_close (File *);
void  file_stat (File *, Stat *);
int   file_read (File *, char *, int);
int   file_write (File *, const char *, int);
