#include "debug.h"
#include "memory.h"
#include "memory/gdt.h"
#include "memory/vmem.h"
#include "memory/palloc.h"


extern char end[];  // defined in `kernel.ld.

static void *ptstart = end;                            // stage 1 start
static void *ptend   = P2V_C (PTESZ * NPDES * NPTES);  // stage 1 end. We are sure the first 4MB is available from bootloader.
static void *phystop = P2V_C (PHYSTOP);                // physical top


/*! Allocate space for page table and switch to the new virutal memory
 * */
void mem_init () {
#ifdef DEBUG
    debug("mem_init: %x:%x, size: %d\n", ptstart, phystop, ptend - phystop);
#endif

    palloc_init (ptstart, ptend);
    kvm_init ();
    gdt_init ();
    palloc_init (ptend, phystop);
}
