#include "string.h"

int memcmp(const void* v1, const void* v2, uint32_t n) {
    const char* s1 = v1;
    const char* s2 = v2;

    while(n --) {
        if (*s1 != *s2) return *s1 - *s2;
        s1++; s2++;
    }
    return 0;
}


void *memmove(void *dest, const void* src, uint32_t n) {
    char* d = dest;
    const char* s = src;

    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (--n) *d-- = *s--;
    } else {
        while (--n) *d++ = *s++;
    }
    return dest;
}


void* memset(void* tgt, int v, uint32_t n) {
    unsigned char *t = tgt;
    while(n--) {
        t[n] = (unsigned char)v;
    }
    return tgt;
}



int strlen(const char*s) {
    int n = 0;
    for (; s[n]; n++);
    return n;
}


int strncmp(const char *s1, const char *s2, uint32_t n) {
    while(n > 0 && *s1 == *s2) {
        n--; s1++; s2++;
    }
    if (n == 0) return 0;
    return *s1 - *s2;
}



char *strncpy(char *dest, const char* src, uint32_t n) {
    while (n-- && (*dest = *src) != 0);
    while (n-- > 0) *dest++ = 0;
    return dest;
}
