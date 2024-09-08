#include "mem.h"

KSTACKSZ equ 16384      ; kernel stack size is 16kb
    global _start
    extern kmain
    extern kstack

; _start is the conventinal elf entry point
; The boot loader already enabled identity paging.
; so we are using virtual address at this point.
_start:
entry:
    ; ; from this point paging is enabled.
    mov esp, (kstack + KSTACKSZ)
    mov eax, kmain
    jmp eax
