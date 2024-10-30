#pragma once
#include <stdint.h>


typedef struct GDTRecord {
    uint16_t  limit;      // size of GDT array - 1
    uint32_t  base;       // pointer to GDT array
} __attribute__((packed)) GDTRecord;

typedef uint64_t GDTEntry;

void     gdt_init ();
GDTEntry create_descriptor (uint32_t base, uint32_t limit, uint16_t flag);
