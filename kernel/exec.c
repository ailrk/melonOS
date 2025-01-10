#include "defs.h"
#include "fdefs.fwd.h"
#include "inode.h"
#include "mem.h"
#include "mmu.h"
#include "string.h"
#include "err.h"
#include "elf.h"
#include "process.h"
#include "process/proc.h"
#include "process/pdefs.h"
#include "memory/vmem.h"
#include "fs/dir.h"

/*  User memory of the program after elf is loaded and stack is setup
 *
 *    +--------------+ KERN_BASE    +--------------+
 *    |   heap       |              |   arg 0      | here are null terminated strings.
 *    +--------------+              |   ...        | we simply store it next to the call stack.
 *    |   stack      | -----------> |   arg N      |
 *    +--------------+              |   0          |
 *    |   guard page |              +--------------+-------------------------- real call stack.
 *    +--------------+              |   &arg 0     | argv[argc]
 *    |   data       |              |   ...        |
 *    +--------------+              |   &arg N     | argv[0]
 *    |   text       |              +--------------+
 *    +--------------+ 0            |   argv       | points to argv[0]
 *                                  +--------------+
 *                                  |   argc       |
 *                                  +--------------+
 *                                  |   0xffffffff | fake ip for return
 *                                  +--------------+
 *                                  |   <empty>    |
 *                                  +--------------+
 *
 * where `data` and `text` section are loaded from the elf, `guard page`, `stack`, and `heap` are
 * allocated by `exec`.
 */

/*! Execute a program */
int exec (char *path, char **argv) {
    Inode             *ino;
    PD                *pgtbl = 0;
    ELF32Header        elfhdr;
    ELF32ProgramHeader ph;
    unsigned           offset;
    size_t             size;
    unsigned           sp;
    unsigned           argc;
    Process           *p;
    PD                *old_pgtbl;

    // load program
    if ((ino = dir_abspath (path, false)) == 0) {
        perror ("exec: invalid path\n");
        return -1;
    }

    if (inode_read (ino, (char *)&elfhdr, 0, sizeof (ELF32Header)) != sizeof (ELF32Header))
        goto bad;

    if (!is_elf (&elfhdr))
        goto bad;

    if ((pgtbl = kvm_allocate ()) == 0)
        goto bad;

    offset = elfhdr.e_phoff;

    for (int i = 0; i < elfhdr.e_phnum; ++i, offset += sizeof (ph)) {
        if (inode_read (ino, (char *)&ph, offset, sizeof (ph)) != sizeof (ph)) {
            goto bad;
        }

        if (ph.p_type != PT_LOAD) {
            continue;
        }

        if (ph.p_memsz < ph.p_filesz) {
            goto bad;
        }

        if (ph.p_vaddr + ph.p_memsz < ph.p_vaddr) {
            goto bad;
        }

        if ((size = uvm_allocate (pgtbl, size, ph.p_vaddr + ph.p_memsz)) == 0) {
            goto bad;
        }

        if (ph.p_vaddr % PAGE_SZ != 0) {
            goto bad;
        }

        if (uvm_load (pgtbl, (char *)ph.p_vaddr, ino, ph.p_offset, ph.p_filesz) == -1) {
            goto bad;
        }
    }

    // don't need inode any more.
    inode_drop(ino);
    ino = 0;

    // allocate 2 pages at the next page boundary,
    // the first page is the guard page and is inaccessible
    // the second page is the user stack
    size = page_alignup(size);
    if ((size = uvm_allocate(pgtbl, size, size + PAGE_SZ * 2)) == 0) {
        goto bad;
    }

    clear_pte_flag(pgtbl, (char *)(size - 2 * PAGE_SZ), PTE_U);

    sp = size;

    // push arguments
    unsigned buffer[3 + MAXARGS + 1];
    for (argc = 0; argv[argc]; ++argc) {
        if (argc > MAXARGS)
            goto bad;
        sp -= strlen(argv[argc]) + 1;
        if (uvm_memcpy (pgtbl, sp, argv[argc], strlen(argv[argc]) + 1) == -1) {
            goto bad;
        }
        buffer[3 + argc] = sp;
    }
    buffer[3 + argc] = 0;
    buffer[2] = sp - (argc + 1) * sizeof(argv[0]);
    buffer[1] = argc;
    buffer[0] = 0xffffffff; // fake return ip
    if (uvm_memcpy(pgtbl, sp, buffer, 3 + argc + 1) == -1) {
        goto bad;
    }

    // go to user space
    p = this_proc();
    strncpy(p->name, path, sizeof(p->name));
    old_pgtbl = p->pgdir;
    p->pgdir = pgtbl;
    p->size = size;
    p->trapframe->eip = elfhdr.e_entry;
    p->trapframe->esp = sp;
    uvm_switch(p);
    vmfree(old_pgtbl);
    return 0;

bad:
    if (pgtbl) {
        vmfree (pgtbl);
    }
    if (ino) {
        inode_drop (ino);
    }
    return -1;
}
