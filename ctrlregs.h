#pragma once

/* CR0 register */
#define CR0_PE (1 << 0)   // 1 = Protected Mode 
#define CR0_WP (1 << 16)  // write protect
#define CR0_AM (1 << 18)  // alignment mask
#define CR0_NW (1 << 29)  // not-write through
#define CR0_CD (1 << 30)  // cache disable
#define CR0_PG (1 << 31)  // 1 = Paging, enable paging and use the § CR3 register.

/* CR4 register */
#define CR4_PSE (1 << 4)  // page size extension
#define CR4_PGE	(1 << 7)  // page Global Enabled
