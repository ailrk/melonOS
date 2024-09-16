#include "ide.h"
#include "i386.h"
#include <stdint.h>
#include "string.h"

/* IDE is the standard interface for hard drives */

#define SECTSZ      512

static inline uint8_t read_status_register() { return inb(IDE_P_7); }

/* Wait for disk ready.
 * inb(0x1f7) getting the status register from IDE PIO mode.
 * 0xc0 => 0x11000000 check DRY (bit 6) and BSY (bit 7) of the
 * disk status.
 *
 * The disk is ready when BSY = 1 && RDY = 0
 * */
void ide_wait_disk() {
    uint8_t mask = IDE_S_RDY | IDE_S_BSY;
    uint8_t ready = IDE_S_RDY;
    while((read_status_register() & mask) != ready);
}


/* read a single sector at offset into dst LBA style. */
void ide_read_sector(void *dst, uint32_t lba) {
    // send command
    ide_wait_disk();
    outb(IDE_P_2, 1);
    outb(IDE_P_3, lba);
    outb(IDE_P_4, lba >> 8);
    outb(IDE_P_5, lba >> 16);
    outb(IDE_P_6, (lba >> 24) | 0xE0);
    outb(IDE_P_7, IDE_C_READSECTOR);

    // read dide
    ide_wait_disk();
    insl(IDE_P_0, dst, SECTSZ/4);     // /4 because insl read words
}


/* read n bytes from lba 0 + offset bytes on the disk to dst
 *
 * The dst buffer size should be greater than (n/SECTSZ)+1.
 * */
void ide_read_offset(void *dst, uint32_t n, uint32_t offset) {
    char *p = dst;
    uint32_t lba = offset / SECTSZ;
    int remained = (n / SECTSZ) + 1;

    {
        char buf[SECTSZ];
        int trash = offset % SECTSZ;
        int rest = SECTSZ - trash;
        ide_read_sector(buf, lba++);
        memcpy(p, &buf[trash], rest);
        remained--;
        p += rest;
    }

    while (remained) {
        ide_read_sector(p, lba++);
        p += SECTSZ;
        remained--;
    }
}
