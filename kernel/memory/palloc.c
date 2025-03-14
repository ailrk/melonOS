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
    log (LOG_BOOT " palloc_init %#x:%#x...\n", vstart, vend);
    debug("1\n");
    pfree_range (vstart, vend);
    log (LOG_BOOT " palloc_init " LOG_OK  "\n");
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


#if DEBUG
void pmem_report() {
    int n = 0;
    if (!kernel_mem.freelist)
        goto end;

    Run *p = kernel_mem.freelist;
    while (p) {
        n++;
        p = p->next;
    }

end:
    debug("pmem: %d pages free \n", n);
}
#endif


/*! free a page of physical memory pointed by v
 *  v needs to align at page boundry otherwise we panic.
 *
 *  @v:  virtual address
 * */
void pfree (char *v) {
    if (V2P_C (v) >= PHYSTOP) {
        #ifdef DEBUG
        debug("pfree: %#x\n", v);
        #endif
        panic ("pfree, physical address exceeds PHYSTOP");
    }

    if (v < end) {
        #ifdef DEBUG
        debug("pfree: va: %#x, end: %#x\n", v, end);
        #endif
        panic ("pfree, invalid virtual address");
    }

    if ((uint32_t)v % PAGE_SZ) {
        #ifdef DEBUG
        debug("pfree: %#x\n", v);
        #endif
        panic ("pfree, address not on page boundry");
    }

    Run* r = (Run *)v;

    r->next = kernel_mem.freelist;
    kernel_mem.freelist = r;
}
