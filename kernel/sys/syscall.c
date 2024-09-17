#include "err.h"
#include "sys/syscall.h"
#include "sys/syscalls.h"
#include "process/proc.h"


int sys_fork() {
    return fork();
}

int sys_exit() {
    exit();
    return 0;
}

int sys_exec() {

}

int sys_sbrk() {

}

int sys_getpid() {
    return this_proc()->pid;
}


static int (* system_calls[])() = {
    [SYS_FORK]   = sys_fork,
    [SYS_EXIT]   = sys_exit,
    [SYS_EXEC]   = sys_exec,
    [SYS_SBRK]   = sys_sbrk,
    [SYS_GETPID] = sys_getpid,
};


static bool is_valid_syscall(int n) {
    return (n > 0 && n < sizeof(system_calls) / sizeof(system_calls[0]) && system_calls[n]);
}


/* Invoke system call from trap frame. The system call needs to parse their
 * own arguments.
 * */
void syscall() {
    Process *p = this_proc();
    if (p == 0) {
        panic("syscall: invalid process");
    }

    int n = p->trapframe->eax;
    if (!is_valid_syscall(n)) {
        p->trapframe->eax = -1;
        perror("known system call");
        return;
    }

    int r = system_calls[n]();
    p->trapframe->eax = r;
}
