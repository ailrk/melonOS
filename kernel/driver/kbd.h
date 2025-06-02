#pragma once
#include <stdint.h>

#define NO              0

/* Special keycodes */
#define K_HOME 0xE0
#define K_END  0xE1
#define K_UP   0xE2
#define K_DN   0xE3
#define K_LF   0xE4
#define K_RT   0xE5
#define K_PGUP 0xE6
#define K_PGDN 0xE7
#define K_INS  0xE8
#define K_DEL  0xE9


/* control `x` */
#define C(x) (x - '@')

typedef uint16_t Scancode;

void kbd_read();
void kbd_drain();
int  kbd_getscancode();
char kbd_translate (Scancode scancode);
