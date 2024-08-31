/* self contained entry for the kernel */

#include "defs.h"
#include "mem.h"
#include "ctrlregs.h"

    global _start
    extern kmain
    extern kstack
    extern page_dir

; _start is the conventinal elf entry point
; we haven't enable paging yet, but the linker is using the 
; virtual address. We need to manually map it to the physical 
; address
_start:
entry:
    ; enable page size extension
    mov eax, cr4
    or eax, CR4_PSE
    mov cr4, eax

    ; setup cr3 to page directory address.
    mov eax, V2P(bootstrap_page_dir)
    mov cr3, eax

    ; enable paging
    mov eax, cr0
    or eax, (CR0_WP | CR0_PG)
    mov cr0, eax

    ; from this point paging is enabled.
    mov esp, (kstack + KSTACKSZ)
    mov eax, kmain
    jmp eax
