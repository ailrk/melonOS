#pragma once

/* Define some virtual address markers 
 *
 * NOTE: this file is used with CPP on the linker file as well, 
 * so please don't add comment after #define because CPP will
 * not remove C comment by itself.
 * */

// Start of extended memory
#define EXTMEM     0x100000

// Top physical memory
#define PHYSTOP    0xE000000

// peripheral device at high address
#define DEV_SPACE  0xFE000000

// separation bettwen ker and user space
#define KERN_BASE  0x80000000

// kernel links here
#define KERN_LINK (KERN_BASE+EXTMEM)

/* address conversion */
#define V2P(addr)  ((addr) - KERN_BASE)
#define P2V(addr)  (((addr)) + KERN_BASE)

/* casted address conversion */
#define V2P_C(addr)  ((uint32_t)(addr) - KERN_BASE)
#define P2V_C(addr)  ((void *)((char *)(addr)) + KERN_BASE)

// page size is 4kb
#define PAGE_SZ    0x1000
