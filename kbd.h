#pragma once
#include "i386.h"

#define KBD_STATP       0x64
#define KBS_DIB         0x01    // keyboard data in buffer
#define KBD_DATA        0x60

// Special keycodes
#define K_HOME          0xE0
#define K_END           0xE1
#define K_UP            0xE2
#define K_DN            0xE3
#define K_LF            0xE4
#define K_RT            0xE5
#define K_PGUP          0xE6
#define K_PGDN          0xE7
#define K_INS           0xE8
#define K_DEL           0xE9



uint32_t kbd_get_status(); 
uint32_t kbd_get_data(); 
uint32_t kbd_getc();
