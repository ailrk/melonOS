#include "err.h"
#include "debug.h"
#include "mem.h"
#include "mmu.h"
#include "string.h"
#include "palloc.h"
#include "memory/pgtbl.h"

extern char end[];

/*! Some untilities for handling page table access
 *
 *  Virtual (va) to physical (pa) address translation process:
 *  1. Take PDE of va from page directory
 *  2. Use PDE of va to get PT from page directory
 * */


PDE *get_pde(PageDir pgdir, const void *vaddr) {
    return &pgdir.t[page_directory_idx((uintptr_t)vaddr)];
}


PageTable get_pt(PageDir pgdir, const void *vaddr) {
    PDE *pde = get_pde (pgdir, vaddr);
    return (PageTable){ .t = (PTE *)P2V(pte_addr (*pde)) };
}


PageTable get_pt1(PDE *pde) {
    return (PageTable){ .t = (PTE *)P2V(pte_addr (*pde)) };
}


PTE *get_pte(PageDir pgdir, const void *vaddr) {
    PageTable pt = get_pt(pgdir, vaddr);
    return &pt.t[page_table_idx((uintptr_t)vaddr)];
}


PTE *get_pte1(PDE *pde, const void *vaddr) {
    PageTable pt = get_pt1(pde);
    return &pt.t[page_table_idx((uintptr_t)vaddr)];
}


/*! Walk the page directory to find the PTE corresponding to the virtual address.
 *  walk will allocate the page table if it doesn't exists.
 *
 *  @pgdir  the page directory
 *  @vaddr  virtual addess
 *  @return the address of the page table entry. 0 indicates failed to find pte
 * */
PTE *walk(PageDir pgdir, const void *vaddr) {
    PDE      *pde = get_pde(pgdir, vaddr);
    PageTable pt;

    if (*pde & PDE_P)
        return get_pte1(pde, vaddr);

    if ((pt.t = (PTE *)palloc()) == 0)
        return 0;

    memset(pt.t, 0, PAGE_SZ);

    *pde = V2P_C((uintptr_t)pt.t | PDE_P | PDE_W | PDE_U);
    return get_pte1(pde, vaddr);
}



/*! Create PTE for virtual addresses starting at vaddr mapped to paddr
 *  virtual address doesn't need to align on page boundry, `map_pages
 *  will automatically round down the address.
 *
 *  @return true if pages are mapped successfully. false otherwise.
 * */
bool map_pages(PageDir pgdir, const VMap *k) {
    int           size   = k->pend - k->pstart;
    char         *vstart = (char *)page_aligndown((uintptr_t)k->virt);
    char         *vend   = (char *)page_aligndown((uintptr_t)k->virt + size);
    physical_addr pstart = k->pstart;
    PTE          *pte;

#if DEBUG && DEBUG_MAP_PAGES
    debug("map_pages> <VMap %#x, (%#x, %#x), %#x>, <vstart %#x, vend %#x>\n",
                 k->virt, k->pstart, k->pend, k->perm, vstart, vend);
#endif

    if ((uintptr_t)vstart % PAGE_SZ > 0 || (uintptr_t)vend % PAGE_SZ > 0)
        panic("map_pages: not on page boundry");

    for (; vstart != vend; vstart += PAGE_SZ, pstart += PAGE_SZ) {
        if ((pte = walk (pgdir, vstart)) == 0)
            return false;

        if (*pte & PTE_P)
            panic ("map_pages: remap");

        *pte = pstart | PTE_P | k->perm;
    }
    return true;
}


/*! Unmap npages from the virtual address mapping k. If free is true then free
 *  the allocation as well. The mapping must exists.
 *  If ptr_add(*pte) is in invalid address range, we simply ignore it.
 * */
void unmap_pages(PageDir pgdir, uintptr_t vstart, size_t n, bool free) {
    uintptr_t vend = vstart + n * PAGE_SZ;
    PTE      *pte;

    if (vstart % PAGE_SZ != 0) {
        panic("unmap_pages: virtual address is not aligned\n");
    }

    for (uintptr_t a = vstart; a < vend; a += PAGE_SZ) {
        if((pte = get_pte(pgdir, (void *)a)) == 0) {
            panic("unmap_pages: get_pte\n");
        }

        if (!(*pte & PTE_P)) {
#if DEBUG && DEBUG_UNMAPPED
            debug("unmap_pages *pte: %#x, a: %#x\n", *pte, a);
            panic("unmap_pages: not mapped\n");
#endif
            goto clear; // ignore unmapped page
        }

        uintptr_t addr = pte_addr(*pte);
        if (addr >= PHYSTOP || (char *)P2V_C(addr) < end) {
#if DEBUG && DEBUG_UNMAPPED
            debug("unmap_pages *pte: %#x, a: %#x\n", *pte, a);
            panic("unmap_pages: outside of PHYSTOP \n");
#endif
            goto clear;
        }

        if (free) {
            uintptr_t addr = pte_addr(*pte);
            pfree(P2V_C(addr));
        }

        clear: *pte = 0;
    }
}



void set_pte_flag(PageDir pgdir, char *vaddr, unsigned flag) {
    if (flag != PTE_P && flag != PTE_W && flag != PTE_U && flag != PTE_D)
        panic("set_pte_flag: unknown flag");

    PTE *pte;
    if ((pte = get_pte(pgdir, vaddr)) == 0) {
        panic("set_pte_flag");
    }
    *pte |= flag;
}


void clear_pte_flag(PageDir pgdir, char *vaddr, unsigned flag) {
    if (flag != PTE_P && flag != PTE_W && flag != PTE_U && flag != PTE_D) {
        panic("clear_pte_flag: unknown flag");
    }
    PTE *pte;
    if ((pte = get_pte(pgdir, vaddr)) == 0) {
        panic("clear_pte_flag");
    }
    *pte &= ~flag;
}
