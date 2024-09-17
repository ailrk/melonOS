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


/* PD and PT entries (PTE):
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


#define VADDR_OFFSET(vaddr)  ((uint32_t)vaddr & 0xfff)

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
#define PG_ADDR(pde, pte, offset) ((uint32_t)((pde) << PD_IDX_SHIFT | (pte) << PT_IDX_SHIFT | (offset)))



/* Segments */

#define SEG_NULL  0     // null descriptor
#define SEG_KCODE 1     // kernel code 
#define SEG_KDATA 2     // kernel data & stack
#define SEG_UCODE 3     // user code
#define SEG_UDATA 4     // user data & stack
#define SEG_TSS   5     // process task state.

#define NSEGS     6

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_S(x)         ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_P(x)         ((x) << 0x07) // Present
#define SEG_AVL(x)       ((x) << 0x0c) // Available for system use
#define SEG_L(x)         ((x) << 0x0d) // Long mode
#define SEG_DB(x)        ((x) << 0x0e) // Size (0 for 16-bit, 1 for 32)
#define SEG_G(x)         ((x) << 0x0f) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_DPL(x)       (((x) &  0x03) << 0x05) // Set privilege level (0 - 3)
 
#define DPL_K              0x00
#define DPL_U              0x03

// Code & data segment type
#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0a // Execute/Read
#define SEG_CODE_EXRDA     0x0b // Execute/Read, accessed
#define SEG_CODE_EXC       0x0c // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0d // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0e // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0f // Execute/Read, conforming, accessed

// System segment type
#define SEG_TSS_32_AVL     0x09
#define SEG_TSS_32_BUSY    0x0b

/*! Task state segment */
typedef struct TaskState {
    uint16_t link;
    uint16_t _padding0;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t _padding1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t _padding2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t _padding3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t _padding4;
    uint16_t cs;
    uint16_t _padding5;
    uint16_t ss;
    uint16_t _padding6;
    uint16_t ds;
    uint16_t _padding7;
    uint16_t fs;
    uint16_t _padding8;
    uint16_t gs;
    uint16_t _padding9;
    uint16_t ldtr;
    uint16_t _padding10;
    uint16_t t;
    uint16_t iobp;
    uint32_t ssp;
} TaskState;