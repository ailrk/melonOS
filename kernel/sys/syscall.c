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
#include "exec.h"
#include "fs/fcntl.h"
#include "sys/syscall.h"
#include "sys/syscalls.h"

/* After `int I_SYSCALL`, the user stack on system call looks
 * like this:
 *
 *      +--------+
 *      |  arg3  |
 *      +--------+
 *      |  arg2  |
 *      +--------+
 *      |  arg1  |
 *      +--------+
 *      |  eip   |
 *      +--------+ <- esp
 *
 * During the system call, the `esp` is saved in the trapframe,
 * the process switch from lower privilege level to high privilege
 * level, so the TSS descriptor we defined earlier will be used to
 * switch the user stack into kernel stack: `ss0` and `esp0` stored in
 * TSS will be used to replace the the current user's `ss` and `esp0`,
 * and user's `ss` and `esp` are stored in the kernel stack.
 * The user stack is still accessible when we are in ring 0. The kernel esp
 * is stored in the trapframe. We need to manually fetch arguments from user
 * stack for each system call.
 * */


/*! Get arguments from user stack.
 *  @nth    the nth argument, starts from 0.
 *  @args   type of arguments
 *            p: pointer
 *            d: int
 *            c: char
 *  @return the pointer points to the nth argument
 * */
void *getarg (size_t nth) {
    Process *thisp = this_proc ();
    void *esp = (void *)thisp->trapframe->esp;
    int offset = sizeof (size_t);

    return esp + offset + (sizeof(size_t) * nth);
}


int getint (size_t nth) { return *(int *)getarg (nth); }


void *getptr (size_t nth) { return *(void **)getarg (nth); }


char getchr (size_t nth) { return *(char *)getarg (nth); }


File *getfile (size_t nth) {
    int fd = getint(nth);
    File *f;
    if (fd < 0)                           return 0;
    if (fd >= NOFILE)                     return 0;
    if ((f = this_proc()->file[fd]) == 0) return 0;
    return f;
}


int sys_fork () {
    return fork ();
}


int sys_exit () {
    exit ();
    return 0;
}


int sys_exec () {
    char *path = (char *)getptr(0);
    char **argv = (char **)getptr(1);
    if (!path) return -1;
    if (!argv) return -1;

    return exec(path, argv);
}


int sys_sbrk () {
    return -1;
}


int sys_getpid () {
    return this_proc()->pid;
}


int sys_read () {
    File        *f    = getfile (0);
    char        *buf  = getptr (1);
    int          sz   = getint (2);

    if (!f)   return -1;
    if (!buf) return -1;
    return file_read (f, buf, sz);
}


int sys_write () {
    File        *f    = getfile (0);
    const void  *buf  = getptr (1);
    int          sz   = getint (2);

    if (!f)   return -1;
    if (!buf) return -1;
    return file_write (f, buf, sz);
}


int sys_mknod () {
    const char  *path  = getptr (0);
    unsigned     major = getint (1);
    unsigned     minor = getint (2);
    Inode       *ino;
    if ((ino = fs_create (path, F_DEV, major, minor)) == 0)
        return -1;
    inode_drop (ino);
    return 0;
}


int sys_mkdir () {
    const char  *path = getptr (0);
    Inode       *ino;
    if ((ino = fs_create (path, F_DIR, 0, 0)) == 0) {
        return -1;
    }
    inode_drop (ino);
    return 0;
}


int sys_open () {
    const char  *path = getptr (0);
    int          mode = getint (1);
    Inode       *ino;
    int          fd;
    File        *f;

    if (mode & O_CREAT) {
        if ((ino = fs_create (path, F_FILE, 0, 0)) == 0) {
            return -1;
        }

    } else {
        if ((ino = dir_abspath (path, false)) == 0) {
            return -1;
        }

        if (!ino->read)
            inode_load (ino);

        if (ino->d.type == F_DIR && mode == O_RDONLY) {
            inode_drop (ino);
            return -1;
        }
    }

    if ((f = file_allocate ()) == 0) {
        inode_drop (ino);
        return -1;
    }

    if ((fd = fs_fdalloc(f)) == -1) {
        file_close (f);
        inode_drop (ino);
        return -1;
    }

    f->type     = FD_INODE;
    f->ino      = ino;
    f->offset   = 0;
    f->readable = !(mode & O_WRONLY);
    f->writable = (mode & O_WRONLY) || (mode & O_RDWR);

    return fd;
}


int sys_close () {
    File *f = getfile (0);

    if (!f) return -1;
    file_close (f);
    return 0;
}


int sys_link () {
    const char  *old  = getptr (0);
    const char  *new  = getptr (1);

    if (!old) return -1;
    if (!new) return -1;

    Inode *ino;
    if ((ino = dir_abspath (old, false)) == 0) {
        return -1;
    }

    if (ino->d.type == F_DIR) {
        return -1;
    }

    ino->d.nlink++;
    inode_flush (ino);

    Inode *dir;
    if ((dir = dir_abspath (new, true)) == 0) {
        goto bad;
    }

    if (dir->dev != ino->dev) {
        goto bad;
    }

    DirEntry new_entry;
    new_entry.inum = ino->inum;
    strncpy (new_entry.name, new, DIRNAMESZ);

    if (!dir_link (dir, new_entry)) {
        goto bad;
    }

    inode_drop (dir);
    return 0;

bad:
    ino->d.nlink--;
    inode_flush (ino);
    return -1;
}


int sys_unlink () {
    return -1;
}


int sys_dup () {
    File *f = getfile (0);

    if (!f) return -1;

    int fd;

    if ((fd = fs_fdalloc(f)) == -1) {
        return -1;
    }
    file_dup(f);
    return fd;
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
    [SYS_DUP]    = sys_dup,
};


static bool is_valid_syscall (unsigned n) {
    return (n > 0 && n < sizeof (system_calls) / sizeof (system_calls[0]) && system_calls[n]);
}


/* Invoke system call from trap frame. The system call needs to parse their
 * own arguments.
 * */
void syscall () {
    Process *p = this_proc ();
    if (p == 0) {
        panic ("syscall: invalid process");
    }

    unsigned n = p->trapframe->eax;
    if (!is_valid_syscall (n)) {
        p->trapframe->eax = -1;
        perror ("known system call");
        return;
    }

    int r = system_calls[n] ();
    p->trapframe->eax = r;
}
