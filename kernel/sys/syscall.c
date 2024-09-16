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


/* Invoke system call from trap frame */
void syscall() {
    Process *p = this_proc();
}
