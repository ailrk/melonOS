#pragma once

#include "fconfig.h"

/* Directory entry */
typedef struct DirEntry {
  unsigned short inum;
  char           name[DIR_SZ];
} DirEntry;
