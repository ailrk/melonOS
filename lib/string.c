#include <stddef.h>
#include "string.h"


int memcmp(const void *v1, const void *v2, size_t n) {
    const char *s1 = v1;
    const char *s2 = v2;

    while(n--) {
        if (*s1 != *s2) return *s1 - *s2;
        s1++; s2++;
    }
    return 0;
}


void *memmove(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;

    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (--n) *d-- = *s--;
    } else {
        while (--n) *d++ = *s++;
    }
    return dest;
}


void memcpy(void *dest, const void *src, size_t n) {
    for (int i = 0; i < n; ++i) {
        ((char *)dest)[i] = ((char *)src)[i];
    }
}


void* memset(void *tgt, int v, size_t n) {
    char *p = (char *)tgt;
    while(n--) {
        *p++ = (unsigned char)v & 0xff;
    }
    return tgt;
}


size_t strlen(const char *s) {
    int n = 0;
    for (; s[n]; n++);
    return n;
}


int strncmp(const char *s1, const char *s2, size_t n) {
    while(n > 0 && *s1 == *s2) {
        n--; s1++; s2++;
    }
    if (n == 0) return 0;
    return *s1 - *s2;
}


char *strncpy(char *dest, const char *src, size_t n) {
    while (n-- && ((*dest++ = *src++) != 0));
    while (n-- > 0) *dest++ = 0;
    return dest;
}


char *strrev(char *s) {
    char *b = s;
    char *e = s;
    char tmp;
    while(*e != '\0') e++;
    e--;
    while(e - b >= 1) {
        tmp = *b;
        *b = *e;
        *e = tmp;
        b++; e--;
    }
    return s;
}
