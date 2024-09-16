/* boot.s
 *
 * Trivia:
 * the space between 0x0 - 0x7c00 is used by BIOS for interrupt vectors,
 * */

ENTRY(boot1)
OUTPUT_FORMAT("binary")  /* output raw binary */


SECTIONS {
    . = 0x7c00;
    .text :
    {
        *(.boot1)
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
}
