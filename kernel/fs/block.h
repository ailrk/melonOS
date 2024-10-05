#pragma once
#include "fdefs.fwd.h"
#include "fs/fdefs.h"


void     block_init(DevNum dev);
void     block_zero(DevNum dev, unsigned blockno);
void     block_super(DevNum dev, SuperBlock *, bool update);
unsigned block_alloc(DevNum dev);
void     block_free(DevNum dev, unsigned blockno);
