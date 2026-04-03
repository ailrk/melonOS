#include "mem.h"

/* This is the user linker.
 *
 * */

ENTRY(_start)


/* ouput elf format */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")


SECTIONS {
    /* ------------------------------
     * User space starts at 1 page above
     * to avoid null pointer
     * */
    . = PAGE_SZ;

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    . = ALIGN(PAGE_SZ);

    .data : {
        *(.data)
        *(.data.*)
    }

    . = ALIGN(PAGE_SZ);

    .bss : {
        *(.bss)
    }

    /* ------------------------------
    * The end of the static memory. This is
    * also page aligned.
     * */
    . = ALIGN(PAGE_SZ);
    PROVIDE(end = .);
}
