#pragma once
#include <stddef.h>
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
PD  *kvm_allocate ();
void kvm_switch ();

void uvm_init (PD *page_dir, char *init, size_t sz);
int  uvm_allocate (PD *page_dir, size_t oldsz, size_t newsz);
int  uvm_deallocate (PD *page_dir, size_t oldsz, size_t newsz);
void uvm_switch (Process *p);
PD  *uvm_copy (PD *page_dir, size_t sz);
void vmfree (PD *);
