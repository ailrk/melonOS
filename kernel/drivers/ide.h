#pragma once
#include <stdint.h>
#include <stddef.h>

#define SECTSZ      512


typedef enum Channel {
    ATA_PRIMARY,
    ATA_SECONDARY
} Channel;



/* ATA Commands */
typedef enum ATACmd {
    ATA_CMD_RD1 = 0x20, // read sector
    ATA_CMD_WT1 = 0x30, // write sector
    ATA_CMD_RDN = 0xc4, // read n sectors
    ATA_CMD_WTN = 0xc5, // write n sectors
} ATACmd;



void ide_wait(Channel ch);
void ide_request(Channel ch, ATACmd cmd, unsigned lba, size_t secn);

void ide_read_request(Channel ch, unsigned lba, size_t secn);
void ide_read(Channel ch, void *dst, size_t secn);

void ide_read_sync(Channel ch, void *dst, unsigned lba, size_t secn);
void ide_write_sync(Channel ch, void *src, unsigned lba, size_t secn);

bool ide_check_error(Channel ch);
