#include "debug.h"
#include "memory.h"
#include "memory/gdt.h"
#include "memory/vmem.h"
#include "memory/palloc.h"


extern char end[];  // defined in `kernel.ld.

static void *ptstart = end;                            // page table start
static void *ptend   = P2V_C (PTESZ * NPDES * NPTES);  // page table end
static void *phystop = P2V_C (PHYSTOP);                // physical top


/*! Allocate space for page table and switch to the new virutal memory */
void mem_init () {
#ifdef DEBUG
    debug("mem_init1: %x:%x, size: %d", ptstart, ptend, ptend - ptstart);
#endif
    palloc_init (ptstart, ptend);
    kvm_init ();
    gdt_init ();
    palloc_init (ptend, phystop);
}
