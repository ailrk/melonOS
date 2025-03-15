#pragma once
#include <stddef.h>
#include "fdefs.fwd.h"
#include "mmu.h"
#include "i386.h"
#include "process/pdefs.h"

/* Virtual addrses mapping */
typedef struct VMap {
    void         *virt;
    physical_addr pstart;
    physical_addr pend;
    int           perm;
} VMap;



void kvm_init ();
bool kvm_allocate (PageDir *pgdir);
void kvm_switch ();

void uvm_init1 (PageDir pgdir, char *init, size_t sz);
int  uvm_allocate (PageDir pgdir, size_t oldsz, size_t newsz);
int  uvm_deallocate (PageDir pgdir, size_t oldsz, size_t newsz);
void uvm_switch (Process *p);
int  uvm_load(PageDir pgdir, char *addr, Inode *ino, unsigned offset, unsigned size);
bool uvm_copy (PageDir pgdir, PageDir *out, size_t sz);
int  uvm_memcpy(PageDir pgdir, unsigned vaddr, void *p, unsigned size);
void vmfree (PageDir);
