#pragma once

#include <stdint.h>

/* Ports */
#define KBP_STAT        0x64 // R   controller status port
#define KBP_DATA        0x60 // R/W data port
#define KBP_CMD         0x64 // W   command port

/* Status port */
#define KBS_OB          0x00 // output buffer status 1=full 0=empty
#define KBS_IB          0x01 // input buffer status  1=full 0=empty


void ps2_init(); 
void ps2out(uint16_t port, uint8_t val);
uint8_t ps2in(uint16_t port);
