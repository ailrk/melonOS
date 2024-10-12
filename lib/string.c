#include <stddef.h>
#include <stdbool.h>
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


void *memset(void *tgt, int v, size_t n) {
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
    while(n > 0 && *s1 && *s2 && *s1 == *s2) {
        n--; s1++; s2++;
    }
    if (n == 0) return 0;
    return *s1 - *s2;
}


char *strncpy(char *dest, const char *src, size_t n) {
    char       *d = dest;
    const char *s = src;
    if (n) {
        while (n--) {
            if ((*d++ = *s++) == 0) {
                while (--n) *d = 0;
                break;
            }
        }
    }
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


static int spn_helper(const char *s, const char *accept, bool c) {
    int n = 0;
    if (accept[0] == '\0') return 0;

    while (*s != '\0') {
        const char *a;
        for (a = accept; *a != '\0'; ++a) {
            if (c && *s != *a) { // strcspn
                n++;
                s++;
                break;
            }

            if (!c && *s == *a) { // strspn
                n++;
                s++;
                break;
            }
        }

        if (*a == '\0') break; // first byte not in accept
    }
    return n;
}


int strspn(const char *s, const char *accept) {
    return spn_helper(s, accept, false);
}


int strcspn(const char *s, const char *accept) {
    return spn_helper(s, accept, true);
}


char *strtok(char *str, const char *delim) {
    static char *old;
    return strtok_r(str, delim, &old);
}


char *strtok_r(char *str, const char *delim, char **saveptr) {
    if (delim[0] == '\0') return 0;

    char *s, *e;

    if (str == 0) s = e = *saveptr;
    else          s = e = str + strspn(str, delim); // skip initial delim

    if (*s == '\0') {
        *saveptr = s;
        return 0;
    }

    e += strcspn(e, delim);

    if (*e == '\0') {
        *saveptr = e;
        return s;
    }

    *e = '\0';
    *saveptr = e + 1;
    return s;
}


char *strpbrk(const char *s, const char *keys) {
    if (!s)    return 0;
    if (!keys) return 0;
    for (; *s != '\0'; ++s) {
        for (const char *k = keys; *k != '\0'; ++k) {
            if (*s == *k) return (char *)s;
        }
    }
    return 0;
}


char *strncat(char *dst, const char *src, size_t n) {
    if (!dst) return 0;
    if (!src) return dst;

    char *p = dst;
    while (*p != '\0') p++;

    for (int i = 0; i < n; ++i)
        p[i] = src[i];

    return dst;
}


char *strchr(const char *s, int chr) {
    char b[2];
    b[0] = chr;
    b[1] = '\0';
    return strpbrk(s, b);
}
