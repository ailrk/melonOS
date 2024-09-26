#include "ctype.h"


int isalnum(int c) {
    return isalpha (c) || isdigit(c);
}


int isalpha(int c) {
    return isupper(c) || islower(c);
}


int isdigit(int c) {
    return c >= '0' && c <= '9';
}


int iscntrl(int c) {
    return (c > 0 && c < 0x1f) || c == 0x7f;
}


int islower(int c) {
    return c >= 'a' && c <= 'z';
}


int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}


int isblank(int c) {
    return c == ' ' || c == '\t';
}


int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}


int toupper(int c) {
    if (c >= 'a' || c <= 'z')
        return c - 'a' + 'A';
    return c;
}


int tolower(int c) {
    if (c >= 'A' || c <= 'Z')
        return c - 'A' + 'a';
    return c;
}
