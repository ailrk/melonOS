#pragma once
#include "i386.h"


#define NO              0

/* Special keycodes */
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


/* Keycode */
#define SHIFT           (1<<0)
#define CTL             (1<<1)
#define ALT             (1<<2)
#define CAPSLOCK        (1<<3)
#define NUMLOCK         (1<<4)
#define SCROLLLOCK      (1<<5)
#define ESC             (1<<6)


/* control `x` */
#define C(x) (x - '@')



void kbd_init();
char kbd_getc();
