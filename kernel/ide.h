#pragma once
#include <stdint.h>

#define SECTSZ      512

#define IDE_S_ERR  0
#define IDE_S_DFE  (1 << 5)
#define IDE_S_RDY  (1 << 6) // 0 when drive is spun down, or after an error.
#define IDE_S_BSY  (1 << 7) // 1 when drive is preparing to send/receive data

#define IDE_P_0  0x1F0

/* LBA mode */
#define IDE_P_1  0x1F1    // always send null byte 0x0
#define IDE_P_2  0x1F2    // (unsigned char) sector count
#define IDE_P_3  0x1F3    // (unsigned char) low 8 bits of LBA
#define IDE_P_4  0x1F4    // (unsigned char) next 8 bits of LBA
#define IDE_P_5  0x1F5    // (unsigned char) next 8 bits of LBA
#define IDE_P_6  0x1F6    // (unsigned char) next 8 bits of LBA
#define IDE_P_7  0x1F7    // send READ SECTORS command

#define IDE_C_READSECTOR   0x20   // read sector
#define IDE_C_WRITESECTOR  0x30   // write sector


void ata_wait_disk();
void ata_read_sector(void *dst, uint32_t lba);
void ata_read_offset(void *dst, uint32_t n, uint32_t offset);
