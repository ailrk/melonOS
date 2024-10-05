#include "memory.h"
#include "memory/gdt.h"
#include "memory/vmem.h"
#include "memory/palloc.h"


void mem_init1() {
    kernel_vmem_init();
    gdt_init();
}


void mem_init2() {
    palloc_init(P2V_C(PTESZ * NPDES * NPTES), P2V_C(PHYSTOP));
}
