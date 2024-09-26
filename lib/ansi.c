#include "ansi.h"
#include "ctype.h"
#include "stdlib.h"
#include "errno.h"

#define BUFSZ 64


static int ansi_buflen(ANSIState *s) {
    return s->buf_top - s->buf;
}


static bool arity_check(ANSIState *s, int arity) {
    return ansi_buflen(s) == arity;
}


static bool set_state(ANSIState *s, char c) {
    switch (c) {
    /* ANSI_CURSOR */
    case 'H':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_MOVE;
        return arity_check(s, 2) || arity_check(s, 0);
    case 'f':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_MOVEX;
        return arity_check(s, 2);
    case 'A':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_UP;
        return arity_check(s, 1);
    case 'B':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_DOWN;
        return arity_check(s, 1);
    case 'C':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_RIGHT;
        return arity_check(s, 1);
    case 'D':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_LEFT;
        return arity_check(s, 1);
    case 'R':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_REP;
        return arity_check(s, 1);
    case 's':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_SAVE;
        return arity_check(s, 0);
    case 'u':
        s->tag          = ANSI_CURSOR;
        s->value.cursor = ANSICursor_RESTORE;
        return arity_check(s, 0);

    /* ANSI_ERASE */
    case 'J':
        s->tag = ANSI_ERASE;
        if (arity_check(s, 0)) {
            s->value.erase = ANSIErase_J;
            return true;
        }
        if (arity_check(s, 1)) {
            int *p = s->buf_top;
            int n  = *--p;
            switch (n) {
            case 0: s->value.erase = ANSIErase_0J; break;
            case 1: s->value.erase = ANSIErase_1J; break;
            case 2: s->value.erase = ANSIErase_2J; break;
            case 3: s->value.erase = ANSIErase_3J; break;
            default: return false;
            }
            return true;
        }
        return false;

    case 'K':
        s->tag = ANSI_ERASE;
        if (arity_check(s, 0)) {
            s->value.erase = ANSIErase_K;
            return true;
        }

        if (arity_check(s, 1)) {
            int *p = s->buf_top;
            int n = *--p;
            switch (n) {
            case 0: s->value.erase = ANSIErase_0K; break;
            case 1: s->value.erase = ANSIErase_1K; break;
            case 2: s->value.erase = ANSIErase_2K; break;
            default: return false;
            }
            return true;
        }
        return false;

    /* ANSI_COLOR */
    case 'm':
        s->tag = ANSI_COLOR;
        if (arity_check(s, 1)) {
            int *p = s->buf_top;
            int n = *--p;
            s->value.color = (ANSIColor)n;
            return true;
        }
        return false;

    default:
        return false;
    }
}


/*! Parse list in form of `n1;n2;..` where nx is an integer.
 *
 *  @top      top pointer of the integer stack.
 *  @cbegin   pointer to start of the string
 *  @return:  return the top pointer of the stack. The pointer points to
 *            one element after the last integral number being parsed.
 * */
static int *parse_list(int *top, const char **cbegin) {
    int n;
    while (*cbegin && ((n = strtol(*cbegin, cbegin)) != 0 || errno != ERANGE)) {
        *top++ = n;
        if (**cbegin == ';') {
            (*cbegin)++;
            continue;
        }
        break;
    }
    return top;
}


/*! Parse an ansi escape code and return the `ANSIState` indicate the escape
 *  code content.
 *
 *  @state:  return value
 *  @c:      start of the escape code string
 *  @size:   max size of the string
 *  @return  the pointer to one plus the last character from the escape code
 * */
const char *ansi_parse(ANSIState *state, const char *c) {
    int *top;
    if (*c++ == '\033' && *c++ == '[') {

        if (isalpha(*c)) {
            return set_state(state, *c++) ? c : 0;
        }

        if (isdigit(*c)) {
            if ((top = parse_list(state->buf, &c)) != 0) {
                state->buf_top = top;
                return set_state(state, *c++) ? c : 0;
            }
        }
   }
    return 0;
}
