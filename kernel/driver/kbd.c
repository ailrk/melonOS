#include <stdbool.h>
#include <stdint.h>
#include "kbd.h"
#include "ps2.h"

#define DEBUG 1


/* Set 1 make code */
#define SC_M_ESC       0x1
#define SC_M_1         0x2
#define SC_M_2         0x3
#define SC_M_3         0x4
#define SC_M_4         0x5
#define SC_M_5         0x6
#define SC_M_6         0x7
#define SC_M_7         0x8
#define SC_M_8         0x9
#define SC_M_9         0xa
#define SC_M_0         0xb
#define SC_M_DASH      0xc
#define SC_M_EQ        0xd
#define SC_M_BSPACE    0xe
#define SC_M_TAB       0xf
#define SC_M_Q         0x10
#define SC_M_W         0x11
#define SC_M_E         0x12
#define SC_M_R         0x13
#define SC_M_T         0x14
#define SC_M_Y         0x15
#define SC_M_U         0x16
#define SC_M_I         0x17
#define SC_M_O         0x18
#define SC_M_P         0x19
#define SC_M_LB        0x1a
#define SC_M_RB        0x1b
#define SC_M_ENTER     0x1c
#define SC_M_LCTL      0x1d
#define SC_M_A         0x1e
#define SC_M_S         0x1f
#define SC_M_D         0x20
#define SC_M_F         0x21
#define SC_M_G         0x22
#define SC_M_H         0x23
#define SC_M_J         0x24
#define SC_M_K         0x25
#define SC_M_L         0x26
#define SC_M_SEMICOL   0x27
#define SC_M_SQUOTE    0x28
#define SC_M_BACKTIP   0x29
#define SC_M_LSHIFT    0x2a
#define SC_M_BSLASH    0x2b
#define SC_M_Z         0x2c
#define SC_M_X         0x2d
#define SC_M_C         0x2e
#define SC_M_V         0x2f
#define SC_M_B         0x30
#define SC_M_N         0x31
#define SC_M_M         0x32
#define SC_M_COMMA     0x33
#define SC_M_PERIOD    0x34
#define SC_M_SLASH     0x35
#define SC_M_RSHIFT    0x36
#define SC_M_KPSTAR    0x37
#define SC_M_LALT      0x38
#define SC_M_SPACE     0x39
#define SC_M_CAPSLCK   0x3a
#define SC_M_F1        0x3b
#define SC_M_F2        0x3c
#define SC_M_F3        0x3d
#define SC_M_F4        0x3e
#define SC_M_F5        0x3f
#define SC_M_F6        0x40
#define SC_M_F7        0x41
#define SC_M_F8        0x42
#define SC_M_F9        0x43
#define SC_M_F10       0x44
#define SC_M_NUMLCK    0x45
#define SC_M_SCROLLOCK 0x46
#define SC_M_KP7       0x47
#define SC_M_KP8       0x48
#define SC_M_KP9       0x49
#define SC_M_KPMINUS   0x4a
#define SC_M_KP4       0x4b
#define SC_M_KP5       0x4c
#define SC_M_KP6       0x4d
#define SC_M_KPPLUS    0x4e
#define SC_M_KP1       0x4f
#define SC_M_KP2       0x50
#define SC_M_KP3       0x51
#define SC_M_KP0       0x52
#define SC_M_KPDOT     0x53
#define SC_M_F11       0x57

#define BREAK(b)        (b | 0x80)


/* Keycode */
#define SHIFT           (1<<0)
#define CTL             (1<<1)
#define ALT             (1<<2)
#define CAPSLOCK        (1<<3)
#define NUMLOCK         (1<<4)
#define SCROLLLOCK      (1<<5)
#define ESC             (1<<6)


char normalmap[256] = {
  NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
  '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
  'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
  '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
  'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
  NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
  NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
  '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
  '\n', '/',
};

char shiftmap[256] = {
  NO, '\033', '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
  '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
  'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
  '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
  'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
  NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
  NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
  '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
};


char capslockmap[256] = {
    [SC_M_A] = 'A',
    [SC_M_B] = 'B',
    [SC_M_C] = 'C',
    [SC_M_D] = 'D',
    [SC_M_E] = 'E',
    [SC_M_F] = 'F',
    [SC_M_G] = 'G',
    [SC_M_H] = 'H',
    [SC_M_I] = 'I',
    [SC_M_J] = 'J',
    [SC_M_K] = 'K',
    [SC_M_L] = 'L',
    [SC_M_M] = 'M',
    [SC_M_N] = 'N',
    [SC_M_O] = 'O',
    [SC_M_P] = 'P',
    [SC_M_Q] = 'Q',
    [SC_M_R] = 'R',
    [SC_M_S] = 'S',
    [SC_M_T] = 'T',
    [SC_M_U] = 'U',
    [SC_M_V] = 'V',
    [SC_M_W] = 'W',
    [SC_M_X] = 'X',
    [SC_M_Y] = 'Y',
    [SC_M_Z] = 'Z',
};


uint16_t modifier = 0;

typedef uint16_t Scancode;

#define SCBUFFER_SZ 256

/* Circular buffer scancode for further consumption */
typedef struct ScancodeBuffer {
    Scancode data[SCBUFFER_SZ];
    uint8_t head;
    uint8_t tail;
} ScancodeBuffer;


ScancodeBuffer sc_buffer = {
    .head = 0,
    .tail = 0
};

bool sc_buffer_empty () { return sc_buffer.head == sc_buffer.tail; }

bool sc_buffer_put (Scancode value) {
    uint8_t tail = (sc_buffer.tail + 1) % SCBUFFER_SZ;
    if (tail == sc_buffer.head) return false;
    sc_buffer.tail = tail;
    sc_buffer.data[sc_buffer.tail] = value;
    return true;
}

bool sc_buffer_get (Scancode *value) {
    if (sc_buffer_empty ()) {
        return false;
    }
    *value = sc_buffer.data[sc_buffer.head % SCBUFFER_SZ];
    sc_buffer.head = (sc_buffer.head + 1)  % SCBUFFER_SZ;
    return true;
}


static bool is_break_code (Scancode scancode) {
    return scancode & 0x80;
}

/*! modify keycode j
 *  @scancode  SET 1 scancode recived from the keyboard.
 *  @return    true if any modifier is updated, false otherwise.
 * */
static bool update_modifier (Scancode scancode) {
    switch (scancode) {
        case SC_M_LSHIFT:
        case SC_M_RSHIFT:
            modifier |= SHIFT;
            break;
        case SC_M_LCTL:
            modifier |= CTL;
            break;
        case SC_M_CAPSLCK:
            if (modifier & CAPSLOCK) {
                modifier &= ~CAPSLOCK;
            } else {
                modifier |= CAPSLOCK;
            }
            break;
        case SC_M_LALT:
            modifier |= ALT;
            break;
        case BREAK(SC_M_LSHIFT):
        case BREAK(SC_M_RSHIFT):
            modifier &= ~SHIFT;
            break;
        case BREAK(SC_M_LCTL):
            modifier &= ~CTL;
            break;
        case BREAK(SC_M_LALT):
            modifier &= ~ALT;
            break;

        default:
            return false;
    }
    return true;
}


/*! Translate scan code into character */
char translate (uint16_t scancode) {
    if (update_modifier (scancode)) {
        return -1;
    }

    if (is_break_code (scancode))
        return -1;

    if (modifier & SHIFT) return shiftmap[scancode];
    if (modifier & CAPSLOCK) return capslockmap[scancode];
    return normalmap[scancode];
}


void kbd_handler () {
    uint8_t data = ps2in (KBP_DATA);
    sc_buffer_put (data);
}


char kbd_getc () {
    uint16_t value;
    if (sc_buffer_get (&value)) {
        return translate (value);
    }
    return -1;
}
