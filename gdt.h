#pragma once

#include <stdint.h>


typedef struct GDTRecord {
    uint16_t  limit;      // size of GDT array - 1
    uint32_t  base;       // pointer to GDT array
} __attribute__((packed)) GDTRecord;


typedef uint64_t GDTEntry; 


#define SEG_NULL  0     // null descriptor
#define SEG_KCODE 1     // kernel code 
#define SEG_KDATA 2     // kernel data & stack
#define SEG_UCODE 3     // user code
#define SEG_UDATA 4     // user data & stack
#define SEG_TSS   5     // process task state.

#define NSEGS     6


void gdt_init();


GDTEntry create_descriptor(uint32_t base, uint32_t limit, uint16_t flag);
