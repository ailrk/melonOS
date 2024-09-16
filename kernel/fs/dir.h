#pragma once

#define DIR_SZ 32

/* Directory entry */
typedef struct DirEntry {
  unsigned short inum;
  char           name[DIR_SZ];
} DirEntry;
