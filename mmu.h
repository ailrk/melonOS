#pragma once
#include <stdint.h>
#include "mem.h"

/* MMU translates virutal address to physical address. We need to setup MMU to
 * enable paging.
 *
 * To map a virutal address to a physical address, we need to look it up from the 
 * page table. A page table structured like a tree with two layers. It contains
 * two parts: PD (page directory) and PT (page table). The root is the PD and leaves are
 * PT.
 *
 * To look up for the physical address, we need to use the virtual address to find the
 * page directory, then locate the page table in that page directory, then finally we 
 * can add offset from the page table address to find teh actual physical address.
 */


/* PD and PT entries (PTE) are the same:
 * 
 * | 31              |12  9|7 8| 6 | 5 | 4  | 3  | 2 | 1 | 0 |
 * | PPN             | AVL |   | D | A | CD | WT | U | W | P |
 *
 *  PPN: physical page number.
 *  D:   dirty
 *  A:   accessed
 *  CD:  cache disabled
 *  WT:  1=write through, 0=write back  
 *  U:   user
 *  U:   writable
 *  P:   present, accessing when it's 0 cause page fault.                     
 */

typedef uint32_t PDE;
typedef uint32_t PTE;


/* PTE flags */
#define PTE_P           0x01   // 1 = present in physical memory
#define PTE_W           0x02   // 1 = read/write, 0 = read only
#define PTE_U           0x04   // 1 = user, 0 = supervisor only
#define PTE_D           0x20   // 1 = dirty, 0 has not been written to


/* PDE flags are the same as PTE except no dirty bit */
#define PDE_P           PTE_P  // 1 = present in physical memory
#define PDE_W           PTE_W  // 1 = read/write, 0 = read only
#define PDE_U           PTE_U  // 1 = user, 0 = supervisor only
#define PDE_PS          0x80   // 1 = 4MB page size, 0 = 4Kb page size


/*
 * For 32-bit linux, e.g PD and PT are both 10 bits long, and each page is 4096 bytes.
 * So in total, PD and PT can index (2**10)**2, or , or 1MB pages, and together with
 * offset it addresses 1MB * 4096, or 4GB memory of a 32bit RAM.
 */


#define PTESZ           4       // size of PTE
#define NPDES           1024    // # directory entries per page directory
#define NPTES           1024    // # PTEs per page table


/* The structure of a 32 bit virtual address:
 * | 10             | 10               |  12     |
 * | Page dir index | page table index |  offset |
 */

#define PT_IDX_SHIFT       12     // offset of PT_IDX in a linear address
#define PD_IDX_SHIFT       22     // offset of PD_IDX in a linear address


/* page directory index */
#define PD_IDX(vaddr)   (((uint32_t)(vaddr) >> PD_IDX_SHIFT) & 0x3FF)

/* page table index */
#define PT_IDX(vaddr)   (((uint32_t)(vaddr) >> PT_IDX_SHIFT) & 0x3FF)


#define PG_ALIGNUP(sz)  (((sz)+PGSIZE-1) & ~(PAGE_SZ-1))
#define PG_ALIGNDOWN(addr) ((addr) & ~(PAGE_SZ-1))


/* Address in page table or page directory entry */
#define PTE_ADDR(pte)   ((unsigned int)(pte) & ~0xFFF)

#define PTE_FLAGS(pte)  ((unsigned int)(pte) &  0xFFF)

/* construct virtual address from indexes and offsets */
#define PG_ADDR(pde, pte, offset) ((uint32_t)((pde) << PDXSHIFT | (pte) << PTXSHIFT | (offset)))
