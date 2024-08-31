#pragma once
#include <stdint.h>

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


#define PTE_P   (1 << 0)
#define PTE_W   (1 << 1)
#define PTE_U   (1 << 2)
#define PTE_A   (1 << 5)
#define PTE_D   (1 << 6)

/*
 * For 32-bit linux, e.g PD and PT are both 10 bits long, and each page is 4096 bytes.
 * So in total, PD and PT can index (2**10)**2, or , or 1MB pages, and together with
 * offset it addresses 1MB * 4096, or 4GB memory of a 32bit RAM.
 *
 * The structure of a 32 bit virtual address:
 * | 10             | 10               |  12     |
 * | Page dir index | page table index |  offset |
 */
