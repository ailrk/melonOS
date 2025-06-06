#pragma once

/* Define some virtual address markers
 *
 * NOTE: this file is used with CPP on the linker file as well,
 * so please don't add comment after #define because CPP will
 * not remove C comment by itself.
 *
 * The virtual memory mapping looks like this:
 *
 *            <Virtual memory>                     <Physical memory>
 *
 *       4GB +------------------+             4GB +------------------+
 *           |  device memory   |                 |  device memory   |
 *           |                  | kmap[3]         |                  |
 * DEV_SPACE +------------------+-----> DEV_SPACE +------------------+
 *
 *               UNUSED                              UNUSED
 *               you can tweak the PHYSTOP and KERN_BASE to change the
 *               amount of memory you can use.
 *
 *           +------------------+----->   PHYSTOP +------------------+
 *           |  free memory     |                 |                  |
 *           |  (palloc)        |                 |                  |
 *       end +------------------+        data end +------------------+ (from linker)
 *           |  kernel  data    |                 |                  |
 *           |                  |  kmap[2]        |                  |
 *      data +------------------+------>     data +------------------+ (from linker)
 *           |kernel text&rodata|  kmap[1]        |                  |
 * KERN_LINK +------------------+----->    EXTMEM +------------------+
 *   (text)  |                  |                 |                  |
 *           |  IO space        |           640k  +------------------+
 *           |                  |  kmap[0]        |                  |
 * KERN_BASE +------------------+ ---------->   0 +------------------+
 *           |                  |
 *           |                  |
 *           |  program data    |
 *           |     & heap       |
 *           |                  |
 *           +------------------+
 *   PAGE_SZ |  user stack      |
 *           +------------------+
 *           |  user data       |
 *           +------------------+
 *           |  user text       |
 *         0 +------------------+
 * */

/* Start of extended memory */
#define EXTMEM    0x100000

/* Max virtual address */
#define MAXVA     0xFFFFFFFF

/* Top physical memory */
#define PHYSTOP   0x20000000

/* peripheral device at high address */
#define DEV_SPACE 0xFE000000

/* separation between ker and user space */
#define KERN_BASE 0x80000000

/* kernel links here */
#define KERN_LINK (KERN_BASE+EXTMEM)

/* address conversion.
 * conversion between kernel address and the physical address.
 * */
#define KA2P(addr)  ((addr) - KERN_BASE)
#define P2KA(addr)  (((addr)) + KERN_BASE)

/* casted address conversion */
#define KA2P_C(addr)  ((uintptr_t)(addr) - KERN_BASE)
#define P2KA_C(addr)  ((void *)((char *)(addr)) + KERN_BASE)

/* page size is 4kb */
#define PAGE_SZ    0x1000
