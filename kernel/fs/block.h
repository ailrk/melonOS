#pragma once
#include "fdefs.fwd.h"
#include "fs/fdefs.h"


void      block_init(devno_t dev);
void      block_zero(devno_t dev, blockno_t blockno);
void      block_super(devno_t dev, SuperBlock *, bool update);
blockno_t block_alloc(devno_t dev);
void      block_free(devno_t dev, blockno_t blockno);
