#include "trap/traps.h"
#include "sys/syscalls.h"

section .init1

global init1
init1:
    ;; int exec (char *path, char **argv)
    push argv
    push path
    push 0  ; where caller ip would be
    mov eax, SYS_EXEC
    int I_SYSCALL

loop: jmp loop


;; init
path:
    db "/init", 0

;; char *argv[] = { init, 0 };
argv:
    db "init", 0
    db 0
