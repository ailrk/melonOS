#include <stdint.h>
#include <stdbool.h>
#include "ide.h"
#include "i386.h"
#include "string.h"
#include "err.h"
#include "tty.h"
#include "spinlock.h"

/* Only support ATA channel for now */


/* ATA Status */
#define ATA_S_ERR  0        // error
#define ATA_S_DFE  (1 << 5) // drive write fault
#define ATA_S_RDY  (1 << 6) // ready. 0=drive is spun down or error.
#define ATA_S_BSY  (1 << 7) // busy. 1=drive is preparing to send/receive

/* ATA Commands */
#define C_RD1SEC  0x20   // read sector
#define C_WT1SEC  0x30   // write sector
#define C_RDNSECS 0xc4   // read n sectors
#define C_WTNSECS 0xc5   // write n sectors

#define C_RD(n) ((n == 1) ? C_RD1SEC : C_RDNSECS)
#define C_WT(n) ((n == 1) ? C_WT1SEC : C_WTNSECS)


typedef enum InterfaceType {
    IDE_ATA,
    IDE_ATAPI   // not supported
} InterfaceType;


typedef enum Drive {
    ATA_MASTER,
    ATA_SLAVE
} Drive;


typedef struct ChannelReg {
    unsigned short base;      // base IO address
    unsigned short ctrl;      // base ctrl address
} ChannelReg;


/* ATA register offset from base. */
typedef enum BaseReg {
    BR_DATA       = 0x00,
    BR_ERROR      = 0x01, // always 0x0
    BR_FEATURES   = 0x01,
    BR_SECN0      = 0x02, // sector count
    BR_LBA0       = 0x03, // lower 8 bytes lba address
    BR_LBA1       = 0x04, // next 8 bytes lba address
    BR_LBA2       = 0x05, // next 8 bytes lba address
    BR_HDDEVSEL   = 0x06, // last 8 bytes lba address
    BR_COMMAND    = 0x07,
    BR_STATUS     = 0x07,
} BaseReg;


/* ATA register offset from ctl. */
typedef enum CtrlReg {
    CR_ALTSTATUS = 0x00, // alternative status register
    CR_DEVCTL    = 0x01, // device ctrl register
    CR_DRIVEADDR = 0x02, // drive address register
} CtrlReg;


typedef enum Error {
    ER_BBK     = 0x80,  // Bad block
    ER_UNC     = 0x40, // Uncorrectable data
    ER_MC      = 0x20, // Media changed
    ER_IDNF    = 0x10, // ID mark not found
    ER_MCR     = 0x08, // Media change request
    ER_ABRT    = 0x04, // Command aborted
    ER_TK0NF   = 0x02, // Track 0 not found
    ER_AMNF    = 0x01, // No address mark
} Error;


static ChannelReg channels[] = {
    [ATA_PRIMARY] = (ChannelReg){
        .base = 0x1f0,
        .ctrl = 0x3f6,
    },
    [ATA_SECONDARY] = (ChannelReg){
        .base = 0x170,
        .ctrl = 0x376
    }
};


inline static unsigned short regb(Channel ch, unsigned short r) {
    return channels[ch].base + r;
}


inline static unsigned short regc(Channel ch, unsigned short r) {
    return channels[ch].ctrl + r;
}


static void print_error(Channel ch, Error e) {
    perror("[IDE] ");
    tty_printf("channel %#x, ", ch);
    switch (e) {
        case ER_BBK:
            perror("Bad block \n");
            return;
        case ER_UNC:
            perror("Uncorrectable data\n");
            return;
        case ER_MC:
            perror("Media change\n");
            return;
        case ER_IDNF:
            perror("ID mark not found\n");
            return;
        case ER_MCR:
            perror("Media change request\n");
            return;
        case ER_ABRT:
            perror("Command aorted\n");
            return;
        case ER_TK0NF:
            perror("Track 0 not found\n");
            return;
        case ER_AMNF:
            perror("No address mark\n");
            return;
        default:
            perror("Unknown error ");
            tty_printf("%d\n", e);
            return;
    }
}


/*! IDE is the standard interface for hard drives */
static inline uint8_t read_status_register(Channel ch) {
    return inb(regb(ch, BR_STATUS));
}


/* !Wait for disk ready.
 *  inb(0x1f7) getting the status register from IDE PIO mode.
 *  0xc0 => 0x11000000 check DRY (bit 6) and BSY (bit 7) of the
 *  disk status.
 *
 *  The disk is ready when BSY = 1 && RDY = 0
 * */
void ide_wait(Channel ch) {
    uint8_t mask = ATA_S_RDY | ATA_S_BSY;
    uint8_t ready = ATA_S_RDY;
    while((read_status_register(ch) & mask) != ready);
}


void ide_request(Channel ch, uint32_t lba, size_t secn) {
    if (secn == 0)
        panic("[IDE] ide_request");
    outb(regb(ch, BR_SECN0), secn);
    outb(regb(ch, BR_LBA0), lba);
    outb(regb(ch, BR_LBA1), lba >> 8);
    outb(regb(ch, BR_LBA2), lba >> 16);
    outb(regb(ch, BR_HDDEVSEL), (lba >> 24) | 0xe0);
}


/*! Read n sectors at offset into dst LBA style.
 *  The size of `dst` should be bigger than (SECTSZ * secn).
 *  @ch   Device channel
 *  @dst  Address read into
 *  @lba  disk address
 *  @secn number of sectors
 * */
void ide_read_secn(Channel ch, void *dst, uint32_t lba, unsigned secn) {
    ide_wait(ch);
    ide_request(ch, lba, secn);
    outb(regb(ch, BR_COMMAND), C_RD(secn));
    ide_wait(ch);
    insl(regb(ch, BR_DATA), dst, SECTSZ/4);     // /4 because insl read words
}


/* read n bytes from lba 0 + offset bytes on the disk to dst
 *
 * The dst buffer size should be greater than (n/SECTSZ)+1.
 * */
void read_offset(Channel ch, void *dst, uint32_t n, uint32_t offset) {
    char *p = dst;
    uint32_t lba = offset / SECTSZ;
    int remained = (n / SECTSZ) + 1;

    char buf[SECTSZ];
    int trash = offset % SECTSZ;
    int rest = SECTSZ - trash;
    ide_read_secn(ch, buf, lba++, 1);
    memcpy(p, &buf[trash], rest);
    remained--;
    p += rest;

    while (remained) {
        ide_read_secn(ch, buf, lba++, 1);
        p += SECTSZ;
        remained--;
    }
}


/* Write n sectors. Size of `src` should be bigger than (secn * SECTSZ)
 * @ch   Device channel
 * @src  memory chunk write to the disk
 * @n    size of memory to write
 * @lba  disk address to write to
 * @secn n sectors per write
 * */
void ide_write_secn(Channel ch, char* src,  uint32_t lba, unsigned secn) {
    ide_wait(ch);
    ide_request(ch, lba, secn);
    outb(regb(ch, BR_COMMAND), C_WT(secn));
    outsl(regb(ch, BR_DATA), src, secn/4);    // /4 because outsl write words.
    ide_wait(ch);
}
