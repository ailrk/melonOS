/* self contained entry for the kernel */

#include "mem.h"

KSTACKSZ equ 16384      ; kernel stack size is 16kb
    global _start
    extern kmain
    extern kstack

; _start is the conventinal elf entry point
; we haven't enable paging yet, but the linker is using the 
; virtual address. We need to manually map it to the physical 
; address
_start:
entry:

    ; ; enable page size extension
    ; mov eax, cr4
    ; or eax, CR4_PSE
    ; mov cr4, eax

    ; ; enable paging
    ; mov eax, cr0
    ; or eax, (CR0_WP | CR0_PG)
    ; mov cr0, eax

    ; ; from this point paging is enabled.
    mov esp, (kstack + KSTACKSZ)
    mov eax, kmain
    jmp eax
