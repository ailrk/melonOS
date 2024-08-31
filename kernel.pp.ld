ENTRY(_start)
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
SECTIONS {
. = (0x80000000 +0x100000);
.text : AT(0x100000)
{
*(.boot)
*(.text)
*(.text.*)
}
PROVIDE(etext = .);
.rodata :
{
*(.rodata)
*(.rodata.*)
}
PROVIDE(erodata = .);
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
. = ALIGN(0x1000);
PROVIDE(data = .);
.data :
{
*(.data)
*(.data.*)
}
PROVIDE(edata = .);
.bss :
{
*(.bss)
}
PROVIDE(end = .);
}
