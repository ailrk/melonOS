#include <stdint.h>
#include <string.h>
#include "err.h"
#include "file.h"
#include "pdefs.h"
#include "process.h"
#include "sys/syscall.h"
#include "sys/syscalls.h"

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
 *  @nth    the nth argument, starts from 1.
 *  @args   type of arguments
 *            p: pointer
 *            d: int
 *            c: char
 *  @return the pointer points to the nth argument
 * */
void *getarg(size_t nth, char *args) {
    if (strlen(args) == 0) {
        panic("getarg");
    }

    if (nth >= strlen(args))
        panic("getarg");

    Process *thisp = this_proc();
    void *esp = (void*)thisp->trapframe->esp;
    int offset = 4;

    while(nth) {
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
        nth--;
    }

    return esp + offset;
}


int getint(size_t nth, char *args) { return *(int *)getarg(nth, args); }


void *getptr(size_t nth, char *args) { return *(void **)getarg(nth, args); }


char getchr(size_t nth, char *args) { return *(char *)getarg(nth, args); }


int sys_fork() {
    return fork();
}


int sys_exit() {
    exit();
    return 0;
}


int sys_exec() {
    return -1;
}


int sys_sbrk() {
    return -1;
}


int sys_getpid() {
    return this_proc()->pid;
}


int sys_read() {
    static char *args = "dpd";
    int          fd   = getint(1, args);
    char        *buf  = (void *)getptr(2, args);
    int          sz   = getint(3, args);
    Process     *p    = this_proc();
    File        *f    = p->file[fd];
    return file_read(f, buf, sz);
}


int sys_write() {
    static char *args = "dpd";
    int          fd   = getint(1, args);
    const void  *buf  = (const void *)getptr(2, args);
    int          sz   = getint(3, args);
    Process     *p    = this_proc();
    File        *f    = p->file[fd];
    return file_write(f, buf, sz);
}


int sys_mknod() {

}


static int (*system_calls[])() = {
    [SYS_FORK]   = sys_fork,
    [SYS_EXIT]   = sys_exit,
    [SYS_EXEC]   = sys_exec,
    [SYS_GETPID] = sys_getpid,
    [SYS_SBRK]   = sys_sbrk,
    [SYS_WRITE]  = sys_write,
    [SYS_READ]   = sys_read,
    [SYS_MKNOD]  = sys_mknod,
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
