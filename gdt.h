#pragma once


#include <stdint.h>


typedef struct GDTRecord {
    uint16_t  limit;      // size of GDT array - 1
    uint32_t  base;       // pointer to GDT array
} __attribute__((packed)) GDTRecord;


#define SEG_KCODE 1     // kernel code 
#define SEG_KDATA 2     // kernel data & stack
#define SEG_UCODE 3     // user code
#define SEG_UDATA 4     // user data & stack
#define SEG_TSS   5     // process task state.

#define NSEGS     6

// Application segment type bits
#define ST_X       0x8     // executable segment
#define ST_W       0x2     // writeable (non-executable segments)
#define ST_R       0x2     // readable (executable segments)

// System segment type bits
#define ST_T32A    0x9     // available 32-bit TSS
#define ST_IG32    0xE     // 32-bit Interrupt Gate
#define ST_TG32    0xF     // 32-bit Trap Gate

/* segment descriptor */
typedef struct GDTEntry {
  unsigned int lim_15_0   : 16; // low bits of segment limit
  unsigned int base_15_0  : 16; // low bits of segment base address
  unsigned int base_23_16 : 8;  // middle bits of segment base address
  unsigned int type       : 4;  // segment type
  unsigned int s          : 1;  // 0 = system, 1 = application
  unsigned int dpl        : 2;  // descriptor Privilege Level
  unsigned int p          : 1;  // present
  unsigned int lim_19_16  : 4;  // high bits of segment limit
  unsigned int avl        : 1;  // unused (available for software use)
  unsigned int rsv1       : 1;  // reserved
  unsigned int db         : 1;  // 0 = 16-bit segment, 1 = 32-bit segment
  unsigned int g          : 1;  // granularity
  unsigned int base_31_24 : 8; // high bits of segment base address
} GDTEntry;



#define create_segment_entry(base, limit, type_, dpl_)      \
    (GDTEntry){                                             \
        .type       = (unsigned int)(type_) & 0xf,          \
        .lim_15_0   = (unsigned int)(limit) & 0xffff,       \
        .lim_19_16  = ((unsigned int)(limit) >> 16) & 0xf,  \
        .base_15_0  = (unsigned int)(base) & 0xffff,        \
        .base_23_16 = ((unsigned int)(base) >> 16) & 0xff,  \
        .base_31_24 = ((unsigned int)(limit) >> 24) & 0xff, \
        .dpl        = (unsigned int)(dpl_) & 0x3,           \
        .s          = 1,                                    \
        .p          = 1,                                    \
        .db         = 1,                                    \
        .g          = 1,                                    \
        .avl        = 0,                                    \
        .rsv1       = 0                                     \
    }                                                       \


 #define DPL_USER    0x3       // User DPL
