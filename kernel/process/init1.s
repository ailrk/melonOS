#include "traps.h"
#include "sys/syscalls.h"

section .init1

global init1
init1:
    mov eax, SYS_GETPID
    int I_SYSCALL
loop: jmp loop

msg:
    db "init1", 0
