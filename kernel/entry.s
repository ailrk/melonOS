#include "defs.h"
    global _start
    extern kmain
    extern kstack

; _start is the conventinal elf entry point
; The boot loader already enabled identity paging.
; so we are using virtual address at this point.
_start:
entry:
    ; 4MB virtual memory should already be avaialable at this point.
    ; The kernel stack should be completely located in [data, end)
    ; We need to make sure (kstack + KSTACKSZ) < end so the kernel stack
    ; doesn't overwrite into the rest of the data section.
    ; this means we need kstack <= end - KSTACKSZ.
    mov esp, (kstack + KSTACK_SZ)
    mov eax, kmain
    jmp eax
