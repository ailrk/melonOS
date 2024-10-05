#pragma once
#include "fdefs.fwd.h"
#include "fs/fdefs.h"


void     block_init(devnum dev);
void     block_zero(devnum dev, blockno blockno);
void     block_super(devnum dev, SuperBlock *, bool update);
unsigned block_alloc(devnum dev);
void     block_free(devnum dev, blockno blockno);
