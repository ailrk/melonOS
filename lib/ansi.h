#pragma once

#include <stdbool.h>


#define ANSI_BUFSZ 64


typedef enum ANSIColor {
    ANSIColor_FBLK = 30, 
    ANSIColor_FRED = 31, 
    ANSIColor_FGRN = 32, 
    ANSIColor_FYLW = 33, 
    ANSIColor_FBLU = 34, 
    ANSIColor_FMAG = 35, 
    ANSIColor_FCYA = 36, 
    ANSIColor_FWHT = 37, 
    ANSIColor_FDEF = 39, 
    ANSIColor_BBLK = 40, 
    ANSIColor_BRED = 41, 
    ANSIColor_BGRN = 42, 
    ANSIColor_BYLW = 43, 
    ANSIColor_BBLU = 44, 
    ANSIColor_BMAG = 45, 
    ANSIColor_BCYA = 46, 
    ANSIColor_BWHT = 47, 
    ANSIColor_BDEF = 49, 
    ANSIColor_RES = 0, 
} ANSIColor;


typedef enum ANSICursor {
    ANSICursor_MOVE,
    ANSICursor_MOVEX,
    ANSICursor_UP,
    ANSICursor_DOWN,
    ANSICursor_RIGHT,
    ANSICursor_LEFT,
    ANSICursor_REP,
    ANSICursor_SAVE,
    ANSICursor_RESTORE,
} ANSICursor;


typedef enum ANSIErase {
    ANSIErase_J,
    ANSIErase_0J,
    ANSIErase_1J,
    ANSIErase_2J,
    ANSIErase_3J,
    ANSIErase_K,
    ANSIErase_0K,
    ANSIErase_1K,
    ANSIErase_2K,
} ANSIErase;


typedef struct ANSIState {
    enum {
        ANSI_CURSOR,
        ANSI_COLOR,
        ANSI_ERASE,
    } tag;

    union {
        ANSIColor color;
        ANSICursor cursor;
        ANSIErase erase;
    } value;

    int buf[ANSI_BUFSZ];
    int *buf_top;

} ANSIState;


const char *ansi_parse(ANSIState *state, const char *c);
