#pragma once
#include <stddef.h>
#include "mmu.h"
#include "i386.h"
#include "process/pdefs.h"

/* Virtual addrses mapping */
typedef struct VMap {
    void *        virt;
    physical_addr pstart;
    physical_addr pend;
    int           perm;
} VMap;


void kernel_vmem_init();
PD * allocate_kernel_vmem();
void switch_kernel_vmem();
int  allocate_user_vmem(PD *page_dir, size_t oldsz, size_t newsz);
int  deallocate_user_vmem(PD *page_dir, size_t oldsz, size_t newsz);
void switch_user_vmem(Process *p);
PD * copy_user_vmem(PD *page_dir, size_t sz);
void init_user_vmem(PD *page_dir, char *init, size_t sz);
void free_vmem(PD *);
