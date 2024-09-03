#include "vmem.h"
#include "defs.h"
#include "err.h"
#include "i386.h"
#include "mem.h"
#include "mmu.h"
#include "palloc.h"
#include <stdbool.h>
#include <stdint.h>
#include "string.h"


extern PDE *kernel_page_dir;
extern uint32_t data;  // defined by kernel.ld

KernelMap kmap[4];


/*! There is one page table on each process. On top of that 
 *  kernel also has it's own page table `kernel_page_dir`.
 * */
static void init_kmap() {
    // IO space
    kmap[0] = (KernelMap)
        { .virt   = (void*)KERN_BASE,
          .pstart = 0,
          .pend   = EXTMEM,
          .perm   = PTE_W
        };

    // kernel text & rodata
    kmap[1] = (KernelMap)
        { .virt   = (void*)KERN_LINK,
          .pstart = V2P_C(KERN_LINK),
          .pend   = V2P_C(data), 
          .perm   = 0
        };

    kmap[2] = (KernelMap)
        // kernel data & memory
        { .virt   = (void*)(data),
          .pstart = V2P_C(data),
          .pend   = PHYSTOP,
          .perm   = PTE_W
        };

    kmap[3] = (KernelMap)
        // devices is mapped to identical address.
        { .virt   = (void*)(DEV_SPACE),
          .pstart = DEV_SPACE,
          .pend   = 0,
          .perm   = PTE_W
        };
}


/*! Return the PTE in page dir corresponding to the virtual address. 
 *
 *  @param page_dir    the page directory
 *  @param vaddr       virtual addess
 *  @param alloc       allocation flag.
 * */
static PTE *get_pte(PDE *page_dir, void *vaddr, bool alloc) {
    PTE *pt;
    PDE *pde = &page_dir[PD_IDX(vaddr)];

    if (*pde & PDE_P) {
        pt = (PTE *)P2V(PTE_ADDR(*pde));
    } else {
        if (!alloc)
            return 0;
        if ((pt = (PTE *)palloc()) == 0)
            return 0;
        memset(pt, 0, PAGE_SZ);
        *pde = V2P((unsigned int)pt | PDE_P | PDE_W | PDE_U);
    }
    return &pt[PT_IDX(vaddr)];
}


/*! Create PTE for virtual addresses starting at vaddr mapped to paddr
 *  virtual address will be round down to the page boundry.
 *
 *  @param page_dir    page directory   
 *  @param vaddr       virtual address
 *  @param size        the size of the page
 *  @param paddr       physical address being mapped to
 *  @param perm        the page permission
 * */
static bool map_pages(PDE *page_dir, void *vaddr, uint32_t size, physical_addr paddr, int perm) {
    char *vbegin = (char *)PG_ALIGNDOWN((uint32_t)vaddr);
    char *vend = (char *)PG_ALIGNDOWN((uint32_t)vaddr + size - 1); 
    PTE *pte;

    for (;;) {
        if ((pte = get_pte(page_dir, vaddr, 1)) == 0)
            return false;
        if (*pte & PTE_P)
            panic("remap");

        *pte = paddr | PTE_P | perm;
        if (vbegin == vend) 
            break;
        vbegin += PAGE_SZ;
        vend += PAGE_SZ;
    }
    return true;
}


/*! setup the kernel part of the page table.
 *
 *  @return initialized page directory
 * */
PDE *setup_kernel_vmem() {
    PDE *page_dir;
    void init_kmap();
    int kmap_sz = sizeof(kmap) / sizeof(kmap[0]) ;

    if ((page_dir = (PDE*)palloc()) == 0)
        return 0;

    memset(page_dir, 0, PAGE_SZ);

    if ((void*)P2V(PHYSTOP) > (void*)DEV_SPACE)
        panic("PHYSTOP is too high");
        
    for (KernelMap *k = kmap; k < &kmap[kmap_sz]; k++) {
        if (map_pages(page_dir, k->virt, k->pend - k->pstart, k->pstart, k->perm)) {
            free_vmem(page_dir);
            return 0;
        }
    }
    return page_dir;
}



/*! setup kernel virtual memory */
void kernel_vmem_alloc() {
    kernel_page_dir = setup_kernel_vmem();
    switch_kernel_vmem();
}


/*! Switch page table register cr3 to kernel only page table. This page table is used
 *  when there is no process running 
 * */
void switch_kernel_vmem() {
    set_cr3(V2P(kernel_page_dir));
}



/*! 
 * */
int deallocate_user_vmem(PDE *page_dir, uint32_t oldsz, uint32_t newsz) {

}


/*! Free a page table.
 *  This will free all memory used in the user part.
 * */
void free_vmem(PDE *page_dir) {
    if (page_dir == 0) 
        panic("free_vmem");
    
    // deallocate
    for (int i = 0; i < NPDES; ++i) {
        if (page_dir[i] & PDE_P) {
            pfree((char *)P2V(PTE_ADDR(page_dir[i])));
        }
    }
    pfree((char *)page_dir);
}
