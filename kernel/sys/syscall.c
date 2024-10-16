#include <stdint.h>
#include "defs.h"
#include "dir.h"
#include "fdefs.h"
#include "inode.h"
#include "proc.h"
#include "string.h"
#include "err.h"
#include "fs.h"
#include "fdefs.fwd.h"
#include "file.h"
#include "pdefs.h"
#include "process.h"
#include "fs/fcntl.h"
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


File *getfile(size_t nth, char *args) {
    int fd = getint(nth, args);
    File *f;
    if (fd < 0)                           return 0;
    if (fd >= NOFILE)                     return 0;
    if ((f = this_proc()->file[fd]) == 0) return 0;
    return f;
}


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
    File        *f    = getfile(1, args);
    char        *buf  = getptr(2, args);
    int          sz   = getint(3, args);

    if (!f)   return -1;
    if (!buf) return -1;
    return file_read(f, buf, sz);
}


int sys_write() {
    static char *args = "dpd";
    File        *f    = getfile(1, args);
    const void  *buf  = getptr(2, args);
    int          sz   = getint(3, args);

    if (!f)   return -1;
    if (!buf) return -1;
    return file_write(f, buf, sz);
}


int sys_mknod() {
    static char *args  = "pdd";
    const char  *path  = getptr(1, args);
    unsigned     major = getint(2, args);
    unsigned     minor = getint(3, args);
    Inode       *ino;
    if ((ino = fs_create(path, F_DEV, major, minor)) == 0)
        return -1;
    inode_drop(ino);
    return 0;
}


int sys_mkdir() {
    static char *args = "p";
    const char  *path = getptr(1, args);
    Inode       *ino;
    if ((ino = fs_create(path, F_DIR, 0, 0)) == 0) {
        return -1;
    }
    inode_drop(ino);
    return 0;
}


int sys_open() {
    static char *args = "pd";
    const char  *path  = getptr(1, args);
    int          mode  = getint(2, args);
    Inode       *ino;
    int          fd;
    File        *f;

    if (mode & O_CREAT) {
        if ((ino = fs_create(path, F_FILE, 0, 0)) == 0) {
            return -1;
        }

    } else {
        if ((ino = dir_abspath(path, false)) == 0) {
            return -1;
        }

        if (!ino->read)
            inode_load(ino);

        if (ino->d.type == F_DIR && mode == O_RDONLY) {
            inode_drop(ino);
            return -1;
        }
    }

    if ((f = file_allocate()) == 0) {
        inode_drop(ino);
        return -1;
    }

    if ((fd = fs_fdalloc(f)) == 0) {
        file_close(f);
        inode_drop(ino);
        return -1;
    }

    f->type     = FD_INODE;
    f->ino      = ino;
    f->offset   = 0;
    f->readable = !(mode & O_WRONLY);
    f->writable = (mode & O_WRONLY) || (mode & O_RDWR);

    return fd;
}


int sys_close() {
    static char *args = "d";
    File *f = getfile(1, args);

    if (!f) return -1;
    file_close(f);
    return 0;
}


int sys_link() {
    static char *args = "pp";
    const char  *old  = getptr(1, args);
    const char  *new  = getptr(2, args);

    if (!old) return -1;
    if (!new) return -1;

    Inode *ino;
    if ((ino = dir_abspath(old, false)) == 0) {
        return -1;
    }

    if (ino->d.type == F_DIR) {
        return -1;
    }

    ino->d.nlink++;
    inode_flush(ino);

    Inode *dir;
    if ((dir = dir_abspath(new, true)) == 0) {
        goto bad;
    }

    if (dir->dev != ino->dev) {
        goto bad;
    }

    DirEntry new_entry;
    new_entry.inum = ino->inum;
    strncpy(new_entry.name, new, DIRNAMESZ);

    if (!dir_link(dir, new_entry)) {
        goto bad;
    }

    inode_drop(dir);
    return 0;

bad:
    ino->d.nlink--;
    inode_flush(ino);
    return -1;
}


int sys_unlink() {
    static char *args = "p";
    return -1;
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
    [SYS_MKDIR]  = sys_mkdir,
    [SYS_OPEN]   = sys_open,
    [SYS_CLOSE]  = sys_close,
    [SYS_LINK]   = sys_link,
    [SYS_UNLINK] = sys_unlink,
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
