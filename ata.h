#pragma once
#include <stdint.h>

#define SECTSZ      512

/* ATA is the standard interface for hard drives */

#define ATA_S_ERR  0
#define ATA_S_DFE  (1 << 5)
#define ATA_S_RDY  (1 << 6) // 0 when drive is spun down, or after an error.
#define ATA_S_BSY  (1 << 7) // 1 when drive is preparing to send/receive data

#define ATA_P_0  0x1F0

/* LBA mode */
#define ATA_P_1  0x1F1    // always send null byte 0x0
#define ATA_P_2  0x1F2    // (unsigned char) sector count
#define ATA_P_3  0x1F3    // (unsigned char) low 8 bits of LBA
#define ATA_P_4  0x1F4    // (unsigned char) next 8 bits of LBA
#define ATA_P_5  0x1F5    // (unsigned char) next 8 bits of LBA
#define ATA_P_6  0x1F6    // (unsigned char) next 8 bits of LBA
#define ATA_P_7  0x1F7    // send READ SECTORS command

#define ATA_C_READSECTOR   0x20   // read sector
#define ATA_C_WRITESECTOR  0x30   // write sector


void wait_disk();
void read_sector(void *dst, uint32_t lba);
void read_offset(void *dst, uint32_t n, uint32_t offset);
