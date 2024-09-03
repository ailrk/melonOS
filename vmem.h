#pragma once

#include "mmu.h"
#include "defs.h"


typedef struct KernelMap {
    void *        virt;
    physical_addr pstart; 
    physical_addr pend;
    int           perm;
} KernelMap;



PDE* setup_kernel_vmem();
void switch_vmem();
void free_vmem(PDE *);
