#include "debug.h"
#include "defs.h"
#include "fdefs.fwd.h"
#include "inode.h"
#include "mem.h"
#include "mmu.h"
#include "pgtbl.h"
#include "string.h"
#include "err.h"
#include "elf.h"
#include "process.h"
#include "process/pdefs.h"
#include "memory/vmem.h"
#include "fs/dir.h"

/* Execute a program */
int exec(char *path, char **argv) {
    Inode             *ino;
    PageDir            pgtbl;
    ELF32Header        elfhdr;
    ELF32ProgramHeader ph;
    unsigned           offset;
    size_t             size = 0;
    unsigned           sp;
    unsigned           argc;
    Process           *p;
    PageDir            old_pgtbl;

    // Load program
    if ((ino = dir_abspath(path, false)) == 0) {
        perror ("exec: invalid path\n");
        return -1;
    }

    // Check ELF
    if (inode_read(ino, (char *)&elfhdr, 0, sizeof(ELF32Header)) != sizeof(ELF32Header))
        goto bad;

    if (!is_elf(&elfhdr))
        goto bad;

    // Make sure kernel space is mapped.
    if (!kvm_allocate(&pgtbl))
        goto bad;

    offset = elfhdr.e_phoff;

    // Load code from ELF program headers.
    for (int i = 0; i < elfhdr.e_phnum; ++i, offset += sizeof(ph)) {
        if (inode_read (ino, (char *)&ph, offset, sizeof(ph)) != sizeof (ph)) {
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

        // Load
        // .text is loaded from 0x1000
        if ((size = uvm_allocate(pgtbl, size, ph.p_vaddr + ph.p_memsz)) == 0) {
            goto bad;
        }

        if (ph.p_vaddr % PAGE_SZ != 0) {
            goto bad;
        }

        if (uvm_load(pgtbl, (char *)ph.p_vaddr, ino, ph.p_offset, ph.p_filesz) == -1) {
            goto bad;
        }
    }

#if DEBUG
    debug("exec> %s ", path);
    for (int i = 0; i < MAXARGS; ++i) {
        if (argv[i]) {
            debug_printf("%s,", argv[i]);
        } else {
            break;
        }
        debug_printf(" ");
    }
    debug_printf("binary loaded\n");
#endif


    // don't need inode any more.
    inode_drop(ino);
    ino = 0;

    // Allocate 2 pages from the next page boundary, the first page is the
    // guard page and is inaccessible the second page is the user stack
    size = page_alignup(size);
    if ((size = uvm_allocate(pgtbl, size, size + PAGE_SZ * 2)) == 0) {
        goto bad;
    }

    // Set guard page permission
    clear_pte_flag(pgtbl, (char *)(size - 2 * PAGE_SZ), PTE_U);

    unsigned buffer[3 + MAXARGS + 1];

    /*
     * We want to build a stack like this:
     *
     *  +-----------------------+
     *  |      0xffffffff       |  buffer[0]: Fake Return IP
     *  +-----------------------+
     *  |         argc          |  buffer[1]: Arg count
     *  +-----------------------+
     *  |     addr of argv[0]   |  buffer[2]: Pointer to argv array
     *  +-----------------------+
     *  |      ptr to arg0      |  buffer[3]: Pointer to string 0
     *  +-----------------------+
     *  |      ptr to arg1      |  buffer[4]: Pointer to string 1
     *  +-----------------------+
     *  |          ...          |
     *  +-----------------------+
     *  |          0            |  buffer[3+argc]: Null terminator
     *  +-----------------------+
     *  |       "arg0\0"        |  Actual string data
     *  +-----------------------+
     *  |       "arg1\0"        |  Actual string data
     *  +-----------------------+
     *  |          ...          |
     *  +-----------------------+  <- sp
     */

    // Set sp to the top.
    sp = size;

    // Clear the buffer
    memset(buffer, 0, sizeof(buffer));

    // Copy args
    for (argc = 0; argv[argc]; ++argc) {
        if (argc > MAXARGS)
            goto bad;
        sp -= strlen(argv[argc]) + 1;

        // copy argv[argc]
        if (uvm_memcpy (pgtbl, sp, argv[argc], strlen(argv[argc]) + 1) == -1) {
            goto bad;
        }
        buffer[3 + argc] = sp;
    }

    // Setup argc and argv.
    buffer[3 + argc] = 0;
    buffer[2] = sp - (argc + 1) * sizeof(argv[0]);
    buffer[1] = argc;
    buffer[0] = 0xffffffff;                   // fake return ip
    sp -= (3 + argc + 1) * sizeof(buffer[0]); // set sp to top


    /* Copy the stack into uvm.
     *    +--------------+ 0
     *    | ...          |
     *    +--------------+
     *    | text         |
     *    +--------------+              +------------+
     *    | data         |              | 0xffffffff |
     *    +--------------+              +------------+
     *    | guard page   |              |            |
     *    +--------------+              |   buffer   |
     *    | stack        | <---cpy----- |            |
     *    +--------------+              |            |
     *    |  heap        |              +------------+
     *    +--------------+ KERN_BASE
     *
     */

    if (uvm_memcpy(pgtbl, sp, buffer, (3 + argc + 1) * sizeof(buffer[0])) == -1) {
        goto bad;
    }

#ifdef DEBUG
    debug("exec> %s, uvm is ready\n", path);
#endif

    p = this_proc();

    // Setup Process.
    strncpy(p->name, path, sizeof(p->name));
    old_pgtbl = p->pgdir;
    p->pgdir = pgtbl;
    p->size = size;

    // Setup the trap frame to jump to e_entry.
    p->trapframe->eip = elfhdr.e_entry;
    p->trapframe->esp = sp;

    // Go to user space
    uvm_switch(p);

    // Now the old page table is no longer needed.
    vmfree(old_pgtbl);
    return 0;

bad:
    if (pgtbl.t) {
        vmfree(pgtbl);
    }
    if (ino) {
        inode_drop(ino);
    }
    return -1;
}
