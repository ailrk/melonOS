#include "defs.h"
#include "dev.h"
#include "trap.h"
#include "fs.h"
#include "process.h"
#include "driver/vga.h"
#include "driver/uart.h"
#include "debug.h"
#include "memory/gdt.h"
#include "memory/vmem.h"
#include "memory/palloc.h"
#include "process/proc.h"


char         kstack[KSTACK_SZ];                        // kernel stack. see entry.s
extern char  end[];                                    // defined in `kernel.ld.
static void *ptstart = end;                            // stage 1 start
static void *ptend   = P2KA_C (PTESZ * NPDES * NPTES);  // stage 1 end. We are sure the first 4MB is available from bootloader.
static void *phystop = P2KA_C (PHYSTOP);                // physical top


void kmain (void) {
    vga_init();

#ifdef DEBUG
    debug("memory: %x:%x, size: %d\n", ptstart, phystop, ptend - phystop);
#endif
    palloc_init (ptstart, ptend);
    kvm_init();
    gdt_init();
    palloc_init(ptend, phystop);
#ifdef DEBUG
    pmem_report();
#endif

    uart_init(COM1);
    uart_init(COM2);
    trap_init();
    dev_init();
    fs_init();

    ptable_init();
    init_pid1();

    scheduler();
}
