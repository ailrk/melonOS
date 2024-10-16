#include "syscalls.h"
#include "traps.h"

;; system call interface from the user space
%macro SYSCALL 2
global %1
%1:
    mov eax, %2
    int I_SYSCALL
    ret
%endmacro

SYSCALL fork,   SYS_FORK
SYSCALL exit,   SYS_EXIT
SYSCALL exec,   SYS_EXEC
SYSCALL getpid, SYS_GETPID
SYSCALL sbrk,   SYS_SBRK
SYSCALL open,   SYS_OPEN
SYSCALL read,   SYS_READ
SYSCALL write,  SYS_WRITE
SYSCALL close,  SYS_CLOSE
SYSCALL mknod,  SYS_MKNOD
SYSCALL mkdir,  SYS_MKDIR
SYSCALL link,   SYS_LINK
SYSCALL unlink, SYS_UNLINK
