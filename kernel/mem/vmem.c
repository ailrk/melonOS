#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "debug.h"
#include "defs.h"
#include "err.h"
#include "gdt.h"
#include "i386.h"
#include "mem.h"
#include "mmu.h"
#include "ncli.h"
#include "pdefs.h"
#include "string.h"
#include "tty.h"
#include "mem/vmem.h"
#include "mem/palloc.h"
#include "process/proc.h"


#define DEBUG 0

PD kernel_page_dir = { .t = 0 };     // kernel only page directory.
extern char data[];                  // defined by kernel.ld

VMap kmap[4];


/*! There is one page table for each process. The following
 *  mapping for kernel space presents on every processes'
 *  page table.
 *
 *      KERN_BASE..KERN_BASE+EXTMEM => 0..EXTMEM
 *      KERN_BASE+EXTMEM..data      => EXTMEM..V2P(data)
 *      data..KERN_BASE+PHYSTOP     => V2P(data)..PHYSTOP
 *      DEV_SPACE..0                => DEV_SPACE..0
 * */
static void init_kmap() {
    // IO space
    kmap[0] = (VMap)
        { .virt   = (void*)KERN_BASE,
          .pstart = 0,
          .pend   = EXTMEM,
          .perm   = PTE_W
        };

    // kernel text & rodata
    kmap[1] = (VMap)
        { .virt   = (void*)KERN_LINK,
          .pstart = V2P_C(KERN_LINK),
          .pend   = V2P_C(data),
          .perm   = 0
        };

    kmap[2] = (VMap)
        // kernel data & memory
        { .virt   = (void*)(data),
          .pstart = V2P_C(data),
          .pend   = PHYSTOP,
          .perm   = PTE_W
        };

    kmap[3] = (VMap)
        // devices is mapped to identical address.
        { .virt   = (void*)(DEV_SPACE),
          .pstart = DEV_SPACE,
          .pend   = 0, // wraps over
          .perm   = PTE_W
        };
}

/*! Some untilities for handling page table access */
static PDE* get_pde(PD *page_dir, const void *vaddr) {
    return &page_dir->t[page_directory_idx((uintptr_t)vaddr)];
}

static PTE* get_pt(PD *page_dir, const void *vaddr) {
    PDE *pde = get_pde(page_dir, vaddr);
    return (PTE *)P2V(pte_addr(*pde));
}

static PTE* get_pt1(PDE *pde) {
    return (PTE *)P2V(pte_addr(*pde));
}

static PTE* get_pte(PD *page_dir, const void *vaddr) {
    PTE *pt = get_pt(page_dir, vaddr);
    return &pt[page_table_idx((uintptr_t)vaddr)];
}

static PTE* get_pte1(PDE *pde, const void *vaddr) {
    PTE *pt = get_pt1(pde);
    return &pt[page_table_idx((uintptr_t)vaddr)];
}


/*! Walk the page directory, return the PTE corresponding to the virtual address.
 *  walk will allocate the page table if it doesn't exists.
 *
 *  @page_dir:  the page directory
 *  @vaddr:     virtual addess
 *  @alloc:     allocation flag.
 *  @return:    the address of the page table entry. 0 indicates failed to find pte
 * */
static PTE *walk(PD *page_dir, const void *vaddr) {
    PDE *pde = get_pde(page_dir, vaddr);
    PTE *pt;

    if (*pde & PDE_P)
        return get_pte1(pde, vaddr);

    if ((pt = (PTE *)palloc()) == 0)
        return 0;

    memset(pt, 0, PAGE_SZ);

    *pde = V2P_C((uintptr_t)pt | PDE_P | PDE_W | PDE_U);
    return get_pte1(pde, vaddr);
}



/*! Create PTE for virtual addresses starting at vaddr mapped to paddr
 *  virtual address doesn't need to align on page boundry, `map_pages
 *  will automatically round down the address.
 *
 *  @return true if pages are mapped successfully. false otherwise.
 * */
static bool map_pages(PD *page_dir, const VMap *k) {
    int           size   = k->pend - k->pstart;
    char *        vstart = (char *)page_aligndown((uintptr_t)k->virt);
    char *        vend   = (char *)page_aligndown((uintptr_t)k->virt + size);
    physical_addr pstart = k->pstart;
    PTE *         pte;

#if DEBUG
    tty_printf("\nmap_pages> <VMap %#x, (%#x, %#x), %#x>", k->virt, k->pstart, k->pend, k->perm);
    tty_printf("\n           <vstart %#x, vend %#x>\n", vstart, vend);
#endif

    if ((uintptr_t)vstart % PAGE_SZ > 0 || (uintptr_t)vend % PAGE_SZ > 0)
        panic("map_pages, not on page boundry");

    for (; vstart != vend; vstart+=PAGE_SZ, pstart+=PAGE_SZ) {
        if ((pte = walk(page_dir, vstart)) == 0)
            return false;

        if (*pte & PTE_P)
            panic("remap");

        *pte = pstart | PTE_P | k->perm;
    }
    return true;
}

/*! Setup the kernel part of the page table. we allocate
 *  a single page to hold the PD. Then we map pages base
 *  on the `kmap` to setup the kernel virtual memory.
 *
 *  @return initialized page directory
 * */
bool allocate_kernel_vmem(PD *page_dir) {
    if ((void *)P2V(PHYSTOP) > (void *)DEV_SPACE)
        panic("PHYSTOP is too high");

    if ((page_dir->t = (PDE*)palloc()) == 0)
        return false;

    memset(page_dir->t, 0, PAGE_SZ);

    int kmap_sz = sizeof(kmap) / sizeof(kmap[0]) ;

    for (VMap *k = kmap; k < &kmap[kmap_sz]; k++) {
        if (!map_pages(page_dir, k)) {
            free_vmem(page_dir);
            return false;
        }
    }
    return true;
}



/*! Switch page table register cr3 to kernel only page table. This page table is used
 *  when there is no process running
 * */
void switch_kernel_vmem() {
   set_cr3(V2P_C(kernel_page_dir.t));
}



/*! Setup kernel virtual memory */
void kernel_vmem_init() {
    tty_printf("[\033[32mboot\033[0m] kernel_vmem_alloc...");
    init_kmap();
    allocate_kernel_vmem(&kernel_page_dir);
    switch_kernel_vmem();
    tty_printf("\033[32mok\033[0m\n");
}


/*! Load init code to address 0 of virtual space.
 *  @page_dir process's page table directory
 *  @init     address of the binary
 *  @sz       size of the binary
 * */
void init_user_vmem(PD *page_dir, char *init, size_t sz) {
    char *mem;
    if (sz > PAGE_SZ)
        panic("\033[32mvmem\033[0m init_user_vmem: more than a page");

    if ((mem = palloc()) == 0)
        panic("\033[32mvmem\033[0m init_user_vmem: not enough memory");

    memset(mem, 0, PAGE_SZ);

    VMap mmap =
        { .virt = 0,
          .pstart = V2P_C(mem),
          .pend = V2P_C(mem + PAGE_SZ),
          .perm = PTE_W | PTE_U
        };
    map_pages(page_dir, &mmap);
    memmove(mem, init, sz);
}


/* Write TSS segment for user space task switching */
static void write_tss(Process *p) {
    push_cli();
    uint16_t flag  = SEG_S(0)       | SEG_P(1)  | SEG_AVL(0) |
                     SEG_L(0)       | SEG_DB(1) | SEG_G(0)   |
                     SEG_DPL(DPL_K) | SEG_TSS_32_AVL;
    uint32_t base  = (uintptr_t)&this_cpu()->ts;
    uint32_t limit = sizeof(this_cpu()->ts) - 1;
    uint8_t  rpl   = 0; // request privilege level
    memset((void*)base, 0, limit);
    this_cpu()->gdt[SEG_TSS] = create_descriptor(base, limit, flag);
    this_cpu()->ts.ss0 = SEG_KDATA << 3;
    this_cpu()->ts.esp0 = (uintptr_t)p->kstack + KSTACK_SZ;
    ltr(SEG_TSS << 3 | rpl);
    pop_cli();
}


/*! Switch TSS and page table to process `p`.
 *  We setup a new TSS entry for the gdt. When interrupt occurs,
 * */
void switch_user_vmem(Process *p) {
    if (!p)
        panic("switch_user_vmem: no process");
    if (!p->kstack)
        panic("switch_user_vmem: no kernel stack");
    if (!p->pgdir.t)
        panic("switch_user_vmem: no page table");

    push_cli();
    write_tss(p); // setup TSS
    set_cr3(V2P_C(this_proc()->pgdir.t));
    pop_cli();
}


/* !Grow process virtual memory from oldsz to newsz, which need not be page
 * aligned. Returns new size or 0 on error.
 * */
int allocate_user_vmem(PD *page_dir, size_t oldsz, size_t newsz) {
    if (newsz > KERN_BASE)
        return 0;

    if (newsz < oldsz)
        return oldsz;

    for (uintptr_t p = page_alignup(oldsz); p < newsz; p += PAGE_SZ) {
        char * mem;
        if ((mem = palloc()) == 0) {
            perror("allocate_user_vmem: out of memory");
            deallocate_user_vmem(page_dir, newsz, oldsz);
            return 0;
        }

        memset(mem, 0, PAGE_SZ);
        VMap mmap =
            { .virt = (char *)p,
              .pstart = V2P_C(mem),
              .pend = V2P_C(mem + PAGE_SZ),
              .perm = PTE_W | PTE_U
            };

        if (!map_pages(page_dir, &mmap)) {
            perror("allocate_user_vmem: out of memory");
            deallocate_user_vmem(page_dir, newsz, oldsz);
            pfree(mem);
            return 0;
        }
    }
    return newsz;
}


/*! Copy the page table of another process.
 *  @page_dir   the page directory of process to copy from.
 *  @new_pgdir  the page directory copy to.
 *  @sz         number of pages copy from the other process.
 *  @return     the copied page directory. 0 if failed.
 * */
bool copy_user_vmem(PD *page_dir, PD *new_pgdir, size_t sz) {
    char *mem;

    if (!(allocate_kernel_vmem(new_pgdir)))
        return false;

    PTE *pte;

    for (size_t i = 0; i < sz; i+=PAGE_SZ) {
        if ((pte = get_pte(page_dir, (void*)i)) == 0)
            panic("copy_user_vmem: trying to copy non existing pte");

        if (!(*pte & PTE_P))
            panic("copy_user_vmem: page not present");

        physical_addr pa = pte_addr(*pte);
        unsigned int flags = pte_flags(*pte);

        if ((mem = palloc()) == 0) {
            free_vmem(new_pgdir);
            return 0;
        }

        memmove(mem, (char *)P2V_C(pa), PAGE_SZ);

        VMap mmap =
            { .virt = (char *)i,
              .pstart = V2P_C(mem),
              .pend = V2P_C(mem + PAGE_SZ),
              .perm = flags
            };
        if (!map_pages(new_pgdir, &mmap)) {
            free_vmem(new_pgdir);
            return 0;
        }
    }
    return true;
}


/* !Shrink process virtual memory from oldsz to newsz, which need not be page
 * aligned. Returns new size.
 * */
int deallocate_user_vmem(PD *page_dir, uintptr_t oldsz, uintptr_t newsz) {
    if (oldsz < newsz)
        return oldsz;

    for (uintptr_t p = page_alignup(newsz); p < oldsz; p += PAGE_SZ) {
        PTE *pte = get_pte(page_dir, (char *)p);

        if (!pte)
            p = page_addr(page_directory_idx(p) + 1, 0, 0) - PAGE_SZ;

        if (*pte & PTE_P) {
            physical_addr page_table_addr = page_table_idx(*pte);
            if (page_table_addr == 0)
                panic("deallocate_user_vmem: trying to pfree invalid physical addr");
            pfree(P2V_C(page_table_addr));
            continue;
        }
    }
    return newsz;
}


/*! Free a page table.
 *  This will free all memory used in the user part.
 * */
void free_vmem(PD *page_dir) {
    if (page_dir == 0)
        panic("free_vmem");

    // deallocate
    deallocate_user_vmem(page_dir, KERN_BASE, 0);
    for (int i = 0; i < NPDES; ++i) {
        if (page_dir->t[i] & PDE_P) {
            pfree((char *)P2V(pte_addr(page_dir->t[i])));
        }
    }
    pfree((char *)page_dir->t);
}
