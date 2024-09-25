#pragma once
#include <stdint.h>

#define SECTSZ      512

typedef enum Channel {
    ATA_PRIMARY,
    ATA_SECONDARY
} Channel;


void ide_wait(Channel ch);
void ide_read_secn(Channel ch, void *dst, unsigned lba, unsigned secn);
void ide_read_offset(Channel ch, void *dst, unsigned n, unsigned offset, unsigned secn);
void ide_write_secn(Channel ch, char* src, unsigned lba, unsigned secn);
