ENTRY(_start)


/* ouput elf format */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")


SECTIONS {
    . = 0x1000; /* User space starts at 1 page above to avoid null pointer */

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .bss : {
        *(.bss)
    }
}
