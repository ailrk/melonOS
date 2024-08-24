/* boot.s should be located at 0x7c00, so the 510th bytes are 0xaa55.
 * CPU will find the bootloader from 0x7c00 automatically.
 * C code are placed after 0xaa55
 * */


ENTRY(boot)              /* the program starts from the boot symbol */
OUTPUT_FORMAT("binary")  /* output raw binary */

SECTIONS {
    . = 0x7c00;
    .text :              /* except .boot and .bss, other secs are from c++ */
    {
        *(.boot)
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
