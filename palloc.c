#include "palloc.h"
#include "mem.h"
#include "err.h"
#include "string.h"
#include <stdint.h>


extern char* end;


/* memory list */
typedef struct Run { struct Run *next; } Run;

typedef struct KernelMem {
    Run *freelist;
} KernelMem;

KernelMem kernel_mem;


/*! free the memory from vstart to vend 
 * */
void pfree_range(void *vstart, void *vend) {
    char *p = (char*)vstart;
    for (; p + PAGE_SZ <= (char*)vend; p += PAGE_SZ)
        pfree(p);
}


/*! free the memory from vstart to vend */
void palloc_init(void *vstart, void *vend) {
    pfree_range(vstart, vend);
}

/*! alloc a PAGE_SZ memory aligned at page boundry
 *  return 0 if the memory cannot be allocated.
 * */
char *palloc() {
    Run *r = kernel_mem.freelist;
    if (r) {
        kernel_mem.freelist = r->next;
    }
    return (char*)r;
}

/*! free a page of physical memory pointed by v 
 *  v needs to align at page boundry otherwise we panic.
 *
 *  @param v  virtual address
 * */
void pfree(char *v) {
    if (V2P_C(v) > PHYSTOP || v < end || (uint32_t)v % PAGE_SZ ) {
        panic("pfree");
    }
    
    // fill page with junks.
    memset(v, 1, PAGE_SZ);

    Run* r = (Run *)v;
    r->next = kernel_mem.freelist; 
    kernel_mem.freelist = r;
}
