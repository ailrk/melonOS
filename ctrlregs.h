#pragma once

/* CR0 register */
#define CR0_PE  0x00000001 // 1 = Protected Mode 
#define CR0_WP  0x00010000 // write protect
#define CR0_PG  0x80000000 // 1 = Paging, enable paging and use the CR3 register.

/* CR4 register */
#define CR4_PSE 0x00000010  // page size extension
#define CR4_PGE	0x00000080  // page Global Enabled
