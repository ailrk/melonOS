#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "string.h"


typedef struct ELF32Header {
    char     e_ident[16];  // elf identification info
    uint16_t e_type;       // object file types
    uint16_t e_machine;    // specify target ISA
    uint32_t e_version;    // 1 or original elf
    uint32_t e_entry;      // memory addr of the entry point
    uint32_t e_phoff;      // points to the start of program header tbl
    uint32_t e_ehoff;      // points to the start of section header tbl
    uint32_t e_flags;      // meaning depends on e_machine
    uint16_t e_ehsize;     // size of header.
    uint16_t e_ephsize;    // size of program header.
    uint16_t e_phnum;      // # of entries in program header tbl
    uint16_t e_shentsize;  // size of section header
    uint16_t e_shnum;      // # of entries in program header tbl
} ELF32Header;


/* ELF32Header.e_ident index */
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8


/* ELF32Header.e_type */
#define ET_NONE	0x00    // unknown.
#define ET_REL	0x01	// relocatable file.
#define ET_EXEC	0x02	// executable file.
#define ET_DYN	0x03	// shared object.
#define ET_CORE	0x04	// core file.


static const char elf_magic[] = { 0x7F, 0x45, 0x4C, 0x46 };


static inline bool is_elf(const ELF32Header *hdr) {
    return memcmp(&hdr->e_ident[0], elf_magic, 4) == 0;
}


typedef struct ELF32ProgramHeader {
    uint32_t p_type;     // type of segment
    uint32_t p_offset;   // offset of the segment in the file image
    uint32_t p_vaddr;    // virtual address of the segment in memory
    uint32_t p_paddr;    // reserved for segment's physical address if relevant
    uint32_t p_filesz;   // size in bytes of the segment in the file image
    uint32_t p_memsz;    // size in bytes of the segment in memory.
    uint32_t p_flags;    // attributes of the program
    uint32_t p_align;    // 0 or 1 means no aligment, otherwise must be power of 2
} ELF32ProgramHeader;


/* ELF32ProgramHeader.p_type */
#define PT_NULL    0x00000000 // program header table entry unused.
#define PT_LOAD    0x00000001 // loadable segment.
#define PT_DYNAMIC 0x00000002 // dynamic linking information.
#define PT_INTERP  0x00000003 // interpreter information.
#define PT_NOTE    0x00000004 // auxiliary information.
#define PT_SHLIB   0x00000005 // reserved.
#define PT_PHDR    0x00000006 // segment containing program header table itself.
#define PT_TLS     0x00000007 // thread-Local Storage template.


/* ELF32ProgramHeader.p_flags */
#define PF_X 0x1 // Executable segment.
#define PF_W 0x2 // Writeable segment.
#define PF_R 0x4 // Readable segment.


typedef struct ELF32SegmentHeader {
    uint32_t sh_name;      // offset to a string in the .shstrtab section
    uint32_t sh_type;      // type of the section
    uint32_t sh_flags;     // attributes of the section.
    uint32_t sh_addr;      // virtual address of the section in memory
    uint32_t sh_offset;    // offset of the section in the file image
    uint32_t sh_size;      // size in bytes of the section
    uint32_t sh_link;      // section index of an associated section
    uint32_t sh_info;      // extra information about the section
    uint32_t sh_addralign; // 0 or 1 means no aligment, otherwise must be power of 2
    uint32_t sh_entsize;   // size, in bytes, of each fixed size entry
} ELF32SegmentHeader;


/* ELF32SegmentHeader.sh_type */
#define SHT_NULL          0x0   // section header table entry unused
#define SHT_PROGBITS      0x1   // program data
#define SHT_SYMTAB        0x2   // symbol table
#define SHT_STRTAB        0x3   // string table
#define SHT_RELA          0x4   // relocation entries with addends
#define SHT_HASH          0x5   // symbol hash table
#define SHT_DYNAMIC       0x6   // dynamic linking information
#define SHT_NOTE          0x7   // notes
#define SHT_NOBITS        0x8   // program space with no data (bss)
#define SHT_REL           0x9   // relocation entries, no addends
#define SHT_SHLIB         0x0A  // reserved
#define SHT_DYNSYM        0x0B  // dynamic linker symbol table
#define SHT_INIT_ARRAY    0x0E  // array of constructors
#define SHT_FINI_ARRAY    0x0F  // array of destructors
#define SHT_PREINIT_ARRAY 0x10  // array of pre-constructors
#define SHT_GROUP         0x11  // section group
#define SHT_SYMTAB_SHNDX  0x12  // extended section indices
#define SHT_NUM           0x13  // number of defined types.
