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

    ; If exec fails, we land here. Clean up the stack.
    add esp, 8

loop: jmp loop


;; init
path:
    db "/init", 0

arg1:
    db "init", 0

;; char *argv[] = { init, 0 };
argv:
    dd arg1, 0
