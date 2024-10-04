#include <stdint.h>
#include <stdbool.h>
#include "ide.h"
#include "i386.h"
#include "defs.h"
#include "err.h"

/* Only support ATA channel for now */


/* ATA Status */
#define ATA_S_ERR  0        // error
#define ATA_S_DFE  (1 << 5) // drive write fault
#define ATA_S_RDY  (1 << 6) // ready. 0=drive is spun down or error.
#define ATA_S_BSY  (1 << 7) // busy. 1=drive is preparing to send/receive


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
    BR_STATUS     = 0x07, // this blocks interrupt.
} BaseReg;


#define HDDEVSEL_CLS 0xa0
#define HDDEVSEL_LBA 0xe0
#define HDDEVSEL_DRIVE(_slave_bit) (_slave_bit << 4)


/* ATA register offset from ctl. */
typedef enum CtrlReg {
    CR_ALTSTATUS = 0x00, // R same as status, doesn't block irq
    CR_DEVCTL    = 0x00, // W device ctrl register
} CtrlReg;


typedef enum Error {
    ER_BBK     = 0x80, // Bad block
    ER_UNC     = 0x40, // Uncorrectable data
    ER_MC      = 0x20, // Media changed
    ER_IDNF    = 0x10, // ID mark not found
    ER_MCR     = 0x08, // Media change request
    ER_ABRT    = 0x04, // Command aborted
    ER_TK0NF   = 0x02, // Track 0 not found
    ER_AMNF    = 0x01, // No address mark
} Error;


ChannelReg channels[] = {
    [ATA_PRIMARY] = (ChannelReg){
        .base = 0x1f0,
        .ctrl = 0x3f6,
    },
    [ATA_SECONDARY] = (ChannelReg){
        .base = 0x170,
        .ctrl = 0x376
    }
};


inline static uint16_t regb(Channel ch, uint8_t r) {
    return channels[ch].base + r;
}


inline static uint16_t regc(Channel ch, uint8_t r) {
    return channels[ch].ctrl + r;
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
    while((inb(regc(ch, CR_ALTSTATUS)) & mask) != ready);
}


/*! Check ide errors */
bool ide_check_error(Channel ch) {
    return inb(regc(ch, CR_ALTSTATUS)) & (ATA_S_ERR | ATA_S_DFE);
}


/*! Send ide command request. This returns immediately so we can
 *  perform disk IO asynchronously.
 *  @ch   Channel
 *  @d    Channel drive
 *  @cmd  ATA command
 *  @lba  LBA address
 *  @secn Sector counts
 * */
void ide_request(Channel ch, Drive d, ATACmd cmd, unsigned lba, size_t secn) {
    if (secn == 0)
        panic("[IDE] ide_request");
    ide_wait(ch);
    outb(regc(ch, CR_DEVCTL)  , 0); // enable interrupts.
    outb(regb(ch, BR_SECN0)   , secn);
    outb(regb(ch, BR_LBA0)    , lba);
    outb(regb(ch, BR_LBA1)    , lba >> 8);
    outb(regb(ch, BR_LBA2)    , lba >> 16);
    outb(regb(ch, BR_HDDEVSEL), (lba >> 24) | HDDEVSEL_LBA | HDDEVSEL_DRIVE(d));
    outb(regb(ch, BR_COMMAND) , cmd);
}


/*! Send read request to ide without waiting.
 *  @ch   Channel
 *  @d    Channel drive
 *  @dst  Memory read to. The size of `dst` should be bigger
 *        than (SECSZ * secn).
 *  @secn read n sectors
 * */
void ide_read_request(Channel ch, Drive d, unsigned lba, size_t secn) {
    ide_wait(ch);
    ATACmd cmd = secn == 1 ? ATA_CMD_RD1 : ATA_CMD_RDN;
    ide_request(ch, d, cmd, lba, secn);
}


/*! Read immediately without sending a read request.
 * */
void ide_read(Channel ch, void *dst, size_t secn) {
    insl(regb(ch, BR_DATA), dst, (SECSZ * secn)/4);
}


/* Write to the disk.
 * @ch   Device channel
 * @d    Channel drive
 * @src  memory chunk write to the disk. Should at least be bigger
 *       than (SECSZ * secn).
 * @secn n sectors per write
 * */
void ide_write_request(Channel ch, Drive d, void* src, unsigned lba, size_t secn) {
    ide_wait(ch);
    ATACmd cmd = secn == 1 ? ATA_CMD_WT1 : ATA_CMD_WTN;
    ide_request(ch, d, cmd, lba, secn);
    // /4 because insl read words
    outsl(regb(ch, BR_DATA), src, (SECSZ * secn)/4);
}
