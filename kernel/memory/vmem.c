#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "debug.h"
#include "log.h"
#include "defs.h"
#include "err.h"
#include "i386.h"
#include "inode.h"
#include "mem.h"
#include "mmu.h"
#include "pdefs.h"
#include "stdlib.h"
#include "string.h"
#include "trap/ncli.h"
#include "memory/vmem.h"
#include "memory/gdt.h"
#include "memory/palloc.h"
#include "memory/pgtbl.h"
#include "process/proc.h"


#define DEBUG_MAP_PAGES 0
#define DEBUG_KVM 1
#define DEBUG_UVM 1


extern char data[];           // defined by kernel.ld
PageDir     kernel_pgdir;     // kernel only page directory.
VMap        kmap[4];


/*! There is one page table for each process. The following
 *  mapping for kernel space presents on every processes'
 *  page table.
 *
 *      KERN_BASE..KERN_BASE+EXTMEM => 0..EXTMEM
 *      KERN_BASE+EXTMEM..data      => EXTMEM..V2P(data)
 *      data..KERN_BASE+PHYSTOP     => V2P(data)..PHYSTOP
 *      DEV_SPACE..0                => DEV_SPACE..0
 * */
static void init_kmap () {
    // IO space
    kmap[0] = (VMap)
        { .virt   = (void *)KERN_BASE,
          .pstart = 0,
          .pend   = EXTMEM,
          .perm   = PTE_W
        };

    // kernel text & rodata
    kmap[1] = (VMap)
        { .virt   = (void *)KERN_LINK,
          .pstart = V2P_C(KERN_LINK),
          .pend   = V2P_C(data),
          .perm   = 0
        };

    kmap[2] = (VMap)
        // kernel data & memory
        { .virt   = (void *)(data),
          .pstart = V2P_C(data),
          .pend   = PHYSTOP,
          .perm   = PTE_W
        };

    kmap[3] = (VMap)
        // devices is mapped to identical address.
        { .virt   = (void *)(DEV_SPACE),
          .pstart = DEV_SPACE,
          .pend   = 0, // wraps over
          .perm   = PTE_W
        };

#ifdef DEBUG
    for (int i = 0; i < sizeof(kmap) / sizeof(kmap[0]); ++i) {
        debug("init_kmap: kmap[%x] va: %#x, pstart: %#x, pend: %#x, perm: %#x\n",
                i, kmap[i].virt, kmap[i].pstart, kmap[i].pend, kmap[i].perm);
    }
#endif
}



/*! Setup the kernel part of the page table. we allocate
 *  a single page to hold the PD. Then we map pages base
 *  on the `kmap` to setup the kernel virtual memory.
 *
 *  @return initialized page directory
 * */
bool kvm_allocate (PageDir *pgdir) {
    if ((void *)P2V (PHYSTOP) > (void *)DEV_SPACE)
        panic ("PHYSTOP is too high");

    if ((pgdir->t = (PD*)palloc ()) == 0)
        return false;

    memset (pgdir->t, 0, PAGE_SZ);

    int kmap_sz = sizeof (kmap) / sizeof (kmap[0]) ;

    for (VMap *k = kmap; k < &kmap[kmap_sz]; k++) {
        if (!map_pages (*pgdir, k)) {
            vmfree (*pgdir);
            return false;
        }
    }

#if DEBUG && DEBUG_KVM
    debug("kvm_allocate: pgdir: %#x\n", pgdir);
#endif

    return true;
}


/*! Switch page table register cr3 to kernel only page table. This page table is used
 *  when there is no process running
 * */
void kvm_switch() {
#if DEBUG && DEBUG_KVM
    debug("kvm_switch\n");
#endif

    set_cr3 (V2P_C (kernel_pgdir.t));
}


/*! Setup kernel virtual memory */
void kvm_init () {
    log (LOG_BOOT " kvm_alloc...\n");
    init_kmap ();
    if (!kvm_allocate (&kernel_pgdir)) {
        panic ("kvm_init");
    }
    kvm_switch ();
    log (LOG_BOOT " kvm_alloc " LOG_OK "\n");
}


/*! Load init1.s code to address 0 of virtual space.
 *  @pgdir process's page table directory
 *  @init  address of the binary
 *  @sz    size of the binary
 * */
void uvm_init1 (PageDir pgdir, char *init, size_t sz) {
#if DEBUG && DEBUG_UVM
    debug("uvm_init, pgdir: %#x, sz: %d\n", pgdir, sz);
#endif

    char *mem;
    if (sz > PAGE_SZ)
        panic ("uvm_init: more than a page");

    if ((mem = palloc ()) == 0)
        panic ("uvm_init: not enough memory");

    memset (mem, 0, PAGE_SZ);

    VMap mmap =
        { .virt   = 0,
          .pstart = V2P_C (mem),
          .pend   = V2P_C (mem + PAGE_SZ),
          .perm   = PTE_W | PTE_U
        };
    map_pages (pgdir, &mmap);
    memmove (mem, init, sz);
}


/* Write TSS segment for user space task switching */
static void write_tss (Process *p) {
    push_cli ();
    uint16_t flag  = SEG_S(0)       | SEG_P(1)  | SEG_AVL(0) |
                     SEG_L(0)       | SEG_DB(1) | SEG_G(0)   |
                     SEG_DPL(DPL_K) | SEG_TSS_32_AVL;
    uint32_t base  = (uintptr_t)&this_cpu()->ts;
    uint32_t limit = sizeof (this_cpu()->ts) - 1;
    uint8_t  rpl   = 0; // request privilege level
    memset((void *)base, 0, limit);
    this_cpu()->gdt[SEG_TSS] = create_descriptor (base, limit, flag);
    this_cpu()->ts.ss0       = SEG_KDATA << 3;
    this_cpu()->ts.esp0      = (uintptr_t)p->kstack + KSTACK_SZ;
    ltr (SEG_TSS << 3 | rpl);
    pop_cli ();
}


/*! Switch TSS and page table to process `p`.
 *  We setup a new TSS entry for the gdt.
 * */
void uvm_switch (Process *p) {
    if (!p)
        panic ("uvm_switch: no process");
    if (!p->kstack)
        panic ("uvm_switch: no kernel stack");
    if (!p->pgdir.t)
        panic ("uvm_switch: no page table");

    push_cli ();
    write_tss (p); // setup TSS
    set_cr3 (V2P_C (this_proc()->pgdir.t));
    pop_cli ();
}


/*! Load a program segment into page directory.
 *  @page_dir page directory
 *  @addr     the address of the program. Must be page aligned
 *  @ino      inode with the start of the program.
 *  @offset   inode offset of the start of the program
 *  @size     program size
 *  @return   0 when succeed, -1 on error
 * */
int uvm_load (PageDir page_dir, char *addr, Inode *ino, unsigned offset, unsigned size) {
    if ((unsigned)addr % PAGE_SZ != 0)
        panic ("uvm_load: address is not aligned");

    PTE *pte;
    physical_addr pa;
    int n;
    for (unsigned i = 0; i < size; i += PAGE_SZ) {
        if ((pte = get_pte (page_dir, addr + i)) == 0) {
            panic ("uvm_load: invalid address");
        }
        pa = pte_addr (*pte);
        n = min (PAGE_SZ, size - i);
        if (inode_read (ino, (char *)P2V (pa), offset + i, n) != n) {
            return -1;
        }
    }
    return 0;
}


/*! Grow process virtual memory from oldsz to newsz, which need not be page
 *  aligned. Returns new size or 0 on error.
 *
 *  The user space address always start from 0, so the returned size can be used
 *  as the end address of the virtual memory.
 * */
int uvm_allocate (PageDir pgdir, size_t oldsz, size_t newsz) {
#if DEBUG && DEBUG_UVM
    debug("uvm_allocate: pgdir: %#x oldsz: %#x, newsz %#x\n", pgdir, oldsz, newsz);
#endif
    if (newsz > KERN_BASE)
        return 0;

    if (newsz < oldsz)
        return oldsz;

    for (uintptr_t p = page_alignup (oldsz); p < newsz; p += PAGE_SZ) {
        char *mem;

        if ((mem = palloc ()) == 0) {
            perror ("uvm_allocate: out of memory (1)\n");
            uvm_deallocate (pgdir, newsz, oldsz);
            return 0;
        }

        memset (mem, 0, PAGE_SZ);
        VMap mmap =
            { .virt   = (char *)p,
              .pstart = V2P_C (mem),
              .pend   = V2P_C (mem + PAGE_SZ),
              .perm   = PTE_W | PTE_U
            };

        if (!map_pages (pgdir, &mmap)) {
            perror ("uvm_allocate: out of memory (2)\n");
            uvm_deallocate (pgdir, newsz, oldsz);
            pfree (mem);
            return 0;
        }
    }
    return newsz;
}


/*! Copy the page table of another process.
 *  @pgdir   the page directory of process to copy from.
 *  @sz      number of pages copy from the other process.
 *  @return  the copied page directory. 0 if failed.
 * */
bool uvm_copy (PageDir pgdir, PageDir *out, size_t sz) {
    char    *mem;
    PageDir  new_pgdir;

    if (!kvm_allocate(&new_pgdir))
        return false;

    PTE *pte;

    for (size_t i = 0; i < sz; i += PAGE_SZ) {
        if ((pte = get_pte (pgdir, (void*)i)) == 0)
            panic("uvm_copy: trying to copy non existed pte");

        if (!(*pte & PTE_P))
            panic("uvm_copy: page not present");

        physical_addr pa    = pte_addr (*pte);
        unsigned      flags = pte_flags (*pte);

        if ((mem = palloc ()) == 0) {
            vmfree (new_pgdir);
            return 0;
        }

        memmove (mem, (char *)P2V_C (pa), PAGE_SZ);

        VMap mmap =
            { .virt   = (char *)i,
              .pstart = V2P_C (mem),
              .pend   = V2P_C (mem + PAGE_SZ),
              .perm   = flags
            };
        if (!map_pages (new_pgdir, &mmap)) {
            vmfree (new_pgdir);
            return 0;
        }
    }
    *out = new_pgdir;
    return true;
}


/* !Shrink process virtual memory from oldsz to newsz, which need not be page
 * aligned. Returns new size.
 * */
int uvm_deallocate (PageDir pgdir, uintptr_t oldsz, uintptr_t newsz) {
    if (oldsz < newsz)
        return oldsz;

    uintptr_t newsz_a = page_alignup(newsz);
    uintptr_t oldsz_a = page_alignup(oldsz);

#if DEBUG
    debug("uvm_deallocate pgdir %#x, oldsz %#x(%#x), newsz %#x(%#x)\n", pgdir, oldsz, oldsz_a, newsz, newsz_a);
#endif

    if (newsz_a < oldsz_a) {
        size_t n = (oldsz_a - newsz_a) / PAGE_SZ;
        unmap_pages(pgdir, newsz_a, n, true);
    }

    return newsz;
}


/*! Map user addrses to kernel address */
static char *uva2ka(PageDir pgdir, char *vaddr) {
    PTE *pte = get_pte(pgdir, vaddr);
    if ((*pte & PTE_P) == 0)
        return 0;
    if ((*pte & PTE_U) == 0)
        return 0;
    return (char *)P2V(pte_addr(*pte));
}


/*! Copy size amount of bytes from address p to user addr in pgdir.
 *  This can be used to copy memory to a different page table.
 *  The page being copied into must have PTE_P and PTE_U set, otherwise
 *  the copy is aborted immediately.
 *  If succeed return 0, otherwise return -1;
 * */
int uvm_memcpy(PageDir pgdir, unsigned vaddr, void *p, unsigned size) {
    char *buf = (char *)p;
    while (size > 0) {
        unsigned va0 = page_aligndown(vaddr);
        char *pa0 = uva2ka(pgdir, (char *)va0);
        if (pa0 == 0)
            return -1;
        unsigned n = PAGE_SZ - (vaddr - va0);
        if (n > size) n = size;
        memmove(pa0 + (vaddr - va0), buf, n);
        size -= n;
        buf += n;
        vaddr = va0 + PAGE_SZ;
    }
    return 0;
}


/*! Free a page table.
 *  This will also free all physical memory used in the user part. (va 0-KERN_BASE)
 * */
void vmfree (PageDir pgdir) {
    debug("vmfree %#x\n", pgdir);
    if (pgdir.t == 0)
        panic ("vmfree");

    // deallocate
    uvm_deallocate (pgdir, KERN_BASE, 0);
    debug("vmfree 1 \n");

    for (int i = 0; i < NPDES; ++i) {
        if (pgdir.t[i] & PDE_P) {
            pfree ((char *)P2V(pte_addr (pgdir.t[i])));
        }
    }
    debug("vmfree 2 \n");

    pfree((char *)pgdir.t);
    debug("vmfree end\n");
}
