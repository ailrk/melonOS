#include "trap/traps.h"
#include "sys/syscalls.h"

section .init1

global init1
init1:
    mov eax, SYS_GETPID
    int I_SYSCALL

    ;; int exec (char *path, char **argv)
    push init
    push argv
    mov eax, SYS_EXEC
    int I_SYSCALL

loop: jmp loop


;; init
init:
    db "init", 0

;; char *argv[] = { init, 0 };
argv:
    db "init", 0
    db 0
