#include "spinlock.h"
#include "fs/buffer.h"
#include "fs/fconfig.h"


typedef struct BCache {
    SpinLock lk;
    BNode *head;
    BNode buffer[NBUF];
} BCache;
