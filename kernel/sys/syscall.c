#include <stdint.h>
#include <string.h>
#include "err.h"
#include "pdefs.h"
#include "sys/syscall.h"
#include "sys/syscalls.h"
#include "process/proc.h"


/* Copying system call arguments from user stack to
 *
 * After `int I_SYSCALL`, the user stack on system call looks
 * like this:
 *      +--------+
 *      |  arg3  |
 *      +--------+
 *      |  arg2  |
 *      +--------+
 *      |  arg1  |
 *      +--------+
 *      |  eip   |
 *      +--------+ <- esp
 * During the system call, the `esp` is saved in the trapframe
 * and TSS switch the stack. We need to manually fetch arguments
 * from the user stack on each sytemcall.
 * */

/*! Get arguments from user stack.
 *  p: pointer
 *  d: int
 *  c: char
 * */
void *getarg(size_t n, char *args) {
    if (n >= strlen(args))
        panic("getarg");

    Process *thisp = this_proc();
    void *esp = (void*)thisp->trapframe->esp;
    int offset = 4;

    while(n) {
        switch (*args++) {
            case 'p':
                offset += sizeof(uintptr_t);
                break;
            case 'd':
                offset += sizeof(int);
                break;
            case 'c':
                offset += sizeof(char);
                break;
            default:
                panic("getarg: unknown arg type");
        }
        n--;
    }

    return esp + offset;
}


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


static bool is_valid_syscall(unsigned n) {
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

    unsigned n = p->trapframe->eax;
    if (!is_valid_syscall(n)) {
        p->trapframe->eax = -1;
        perror("known system call");
        return;
    }

    int r = system_calls[n]();
    p->trapframe->eax = r;
}
