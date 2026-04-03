#include "mem.h"

/* This is the kernel linker.
 * If you are looking for bootloader linker, it's in boot/bootld.ld
 * If you are looking for user space linker, checkut melon/user.ld
 *
 * There is a distinction between VMA (Virutal memory address)
 * and LMA (Load memory address).
 *
 * The VMA is where the code expects to run. When setting the local
 * counter `.` to KERN_LINK, it tells the compiler to Generate all
 * jump instructions and pointer addresses assuming this code lives
 * at the KERN_LINK+ mark.
 *
 * The LMA is where the code is actually stored. `.text : AT(EXTMEM)`
 * tells the linker: "Even though this code thinks it lives at KERN_LINK,
 * which is a virtual address, physically put the bytes at EXTMEM." This
 * is how we establish the mapping.
 *
 * PROVIDE adds a label to the address in elf.
 * AT sets the physical memory address.
 * ALIGN align to the next page.
 * */

ENTRY(_start)

/* ouput elf format */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")


SECTIONS {
    /* ----------------------------------------
     * Kernel base + extended memory. kernel text
     * starts from here.
     * */
    . = KERN_LINK;

    .text : AT(EXTMEM) /* Physicall put it on EXTMEM */
    {
        *(.boot)
        *(.text)
        *(.text.*)
    }

    PROVIDE(etext = .);	/* End of text */

    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    }
    PROVIDE(erodata = .);	/* end of rodata */

    /* ----------------------------------------
     * Debugging information in kernel memory
     * */
	.stab : {
		PROVIDE(__STAB_BEGIN__ = .);
		*(.stab);
		PROVIDE(__STAB_END__ = .);
	}

	.stabstr : {
		PROVIDE(__STABSTR_BEGIN__ = .);
		*(.stabstr);
		PROVIDE(__STABSTR_END__ = .);
	}

    /* ----------------------------------------
     * Source of init1.s is hard wired into the
     * kernel text sapce. It's put right after
     * the debugging info and before .data.
     *
     * Other user space executables need to be
     * read from the disk.
     * */
    .init1 : {
		PROVIDE(__INIT1_BEGIN__ = .);
		*(.init1);
		PROVIDE(__INIT1_END__ = .);
	}

    /* ----------------------------------------
     * The start of the kernel .data and .bss
     * section. This section starts with `data`
     * and ends with `end`.
     *
     * Palloc relies on accessing memory from
     * `end`.
     * */

    /* Align data segment to the next page
     * Aligning .data  and .bss to page boundry
     * so we can set their access permission
     * when setting up the page table.
     *
     * .text should be read only and executable,
     * .data/.bss should be read-write and not
     * executable.
     *
     * If we don't align it here, the end of the
     * code and the start of the data can share
     * the same page which shares permission.
     * */
    . = ALIGN(PAGE_SZ);

    PROVIDE(data = .);

    .data :
    {
        *(.data)
        *(.data.*)
    }

    PROVIDE(edata = .);  /* End of .data */

    . = ALIGN(PAGE_SZ);

    .bss :
    {
        *(.bss)
    }


    /* ----------------------------------------
     * The end of data section.
     *
     * The size of the data-end section is determined
     * at link time and stay constant as the kernel
     * runs.
     * */
    . = ALIGN(PAGE_SZ);
    PROVIDE(end = .);


    /* ----------------------------------------
     * After this are free memory from palloc.
     */
}
