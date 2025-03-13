#include <stdint.h>
#include "debug.h"
#include "log.h"
#include "palloc.h"
#include "mem.h"
#include "err.h"

extern char end[];


/* memory list */
typedef struct KernelMem {
    Run *freelist;
} KernelMem;


KernelMem kernel_mem;


/*! free the memory in range [vstart, vend) */
void pfree_range (void *vstart, void *vend) {
    char *p = (char *)vstart;
    for (; p + PAGE_SZ <= (char *)vend; p += PAGE_SZ) {
        pfree (p);
    }
}


/*! free the memory from vstart to vend */
void palloc_init (void *vstart, void *vend) {
    log ("[\033[32mboot\033[0m] palloc_init...");
    pfree_range (vstart, vend);
    log ("\033[32mok\033[0m\n");
}


/*! alloc a PAGE_SZ memory aligned at page boundry
 *  return 0 if the memory cannot be allocated.
 * */
char *palloc () {
    Run *r = kernel_mem.freelist;

    if (r) {
        kernel_mem.freelist = r->next;
    } else {
        perror ("palloc: no memory available\n");
    }
    return (char *)r;
}


/*! free a page of physical memory pointed by v
 *  v needs to align at page boundry otherwise we panic.
 *
 *  @v:  virtual address
 * */
void pfree (char *v) {
    if (V2P_C (v) >= PHYSTOP) {
        #ifdef DEBUG
        debug("pfree: %#08x\n", v);
        #endif
        panic ("pfree, physical address exceeds PHYSTOP");
    }

    if (v < end) {
        #ifdef DEBUG
        debug("pfree: va: %#08x, end: %#08x\n", v, end);
        #endif
        panic ("pfree, invalid virtual address");
    }

    if ((uint32_t)v % PAGE_SZ) {
        #ifdef DEBUG
        debug("pfree: %#08x\n", v);
        #endif
        panic ("pfree, address not on page boundry");
    }

    Run* r = (Run *)v;

    r->next = kernel_mem.freelist;
    kernel_mem.freelist = r;
}
