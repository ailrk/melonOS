#include "mem.h"

ENTRY(_start)

/* ouput elf format */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")


SECTIONS {
    /* kernel base + extended memory. kernel text starts from here*/
    . = KERN_LINK;

    .text : AT(EXTMEM)
    {
        *(.boot)
        *(.text)
        *(.text.*)
    }

    PROVIDE(etext = .);	/* end of text */

    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    }
    PROVIDE(erodata = .);	/* end of rodata */

    /* debugging information in kernel memory */
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

    /* init1.s */
    .init1 : {
		PROVIDE(__INIT1_BEGIN__ = .);
		*(.init1);
		PROVIDE(__INIT1_END__ = .);
	}

    /* align data segment to the next page */
    . = ALIGN(PAGE_SZ);

    PROVIDE(data = .);

    .data :
    {
        *(.data)
        *(.data.*)
    }

    PROVIDE(edata = .);  /* end of data */

    . = ALIGN(PAGE_SZ);
    .bss :
    {
        *(.bss)
    }

    . = ALIGN(PAGE_SZ);
    PROVIDE(end = .);    /* the end */
}
