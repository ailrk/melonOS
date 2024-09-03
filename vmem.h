#pragma once

#include "mmu.h"
#include "defs.h"


typedef struct KernelMap {
    void *        virt;
    physical_addr pstart; 
    physical_addr pend;
    int           perm;
} KernelMap;



void kernel_vmem_alloc();
PDE *setup_kernel_vmem();
void switch_kernel_vmem();
int  deallocate_user_vmem(PDE *page_dir, uint32_t oldsz, uint32_t newsz);
void free_vmem(PDE *page_dir);
