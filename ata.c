#include "ata.h"
#include "i386.h"
#include <stdint.h>
#include "tty.h"
#include "string.h"

#define SECTSZ      512

static inline uint8_t read_status_register() { return inb(ATA_P_7); }

/* Wait for disk ready. 
 * inb(0x1f7) getting the status register from ATA PIO mode.
 * 0xc0 => 0x11000000 check DRY (bit 6) and BSY (bit 7) of the 
 * disk status.
 *
 * The disk is ready when BSY = 1 && RDY = 0
 * */
void wait_disk() {
    uint8_t mask = ATA_S_RDY | ATA_S_BSY; 
    uint8_t ready = ATA_S_RDY;
    while((read_status_register() & mask) != ready);
}


/* read a single sector at offset into dst LBA style. */
void read_sector(void *dst, uint32_t lba) {
    // send command
    wait_disk();
    outb(ATA_P_2, 1);
    outb(ATA_P_3, lba);
    outb(ATA_P_4, lba >> 8);
    outb(ATA_P_5, lba >> 16);
    outb(ATA_P_6, (lba >> 24) | 0xE0);
    outb(ATA_P_7, ATA_C_READSECTOR);
    
    // read data
    wait_disk();
    insl(ATA_P_0, dst, SECTSZ/4);     // /4 because insl read words
}


/* read n bytes from lba 0 + offset bytes on the disk to dst
 *
 * The dst buffer size should be greater than (n/SECTSZ)+1.
 * */
void read_offset(void *dst, uint32_t n, uint32_t offset) {
    char *p = dst;
    uint32_t lba = offset / SECTSZ;
    int remained = (n / SECTSZ) + 1;

    {
        char buf[SECTSZ];
        int trash = offset % SECTSZ;
        int rest = SECTSZ - trash;
        read_sector(buf, lba++);
        memcpy(p, &buf[trash], rest);
        remained--;
        p += rest;
    }

    while (remained) {
        read_sector(p, lba++);
        p += SECTSZ;
        remained--;
    }
}
