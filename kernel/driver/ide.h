#pragma once
#include <stdbool.h>
#include <stddef.h>


typedef enum Channel {
    ATA_PRIMARY,
    ATA_SECONDARY
} Channel;


typedef enum Drive {
    ATA_MASTER,
    ATA_SLAVE
} Drive;


/* ATA Commands */
typedef enum ATACmd {
    ATA_CMD_RD1 = 0x20, // read sector
    ATA_CMD_WT1 = 0x30, // write sector
    ATA_CMD_RDN = 0xc4, // read n sectors
    ATA_CMD_WTN = 0xc5, // write n sectors
} ATACmd;


void ide_wait(Channel ch);
void ide_request(Channel ch, Drive d, ATACmd cmd, unsigned lba, size_t secn);
void ide_read_request(Channel ch, Drive d, unsigned lba, size_t secn);
void ide_read(Channel ch, void *dst, size_t secn);
void ide_write_request(Channel ch, Drive d, void *src, unsigned lba, size_t secn);
bool ide_check_error(Channel ch);
bool ide_has_secondary(Channel ch);
