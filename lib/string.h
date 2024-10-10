#pragma once
#include <stddef.h>


int    memcmp(const void *, const void *, size_t);
void  *memmove(void *, const void *, size_t);
void   memcpy(void *, const void *, size_t);
void  *memset(void *, int, size_t);
size_t strlen(const char *);
int    strncmp(const char *, const char *, size_t);
char  *strncpy(char *, const char *, size_t);
char  *strrev(char *s);
char  *strtok(char *str, const char *delim);
char  *strtok_r(char *str, const char *delim, char **saveptr);
int    strspn(const char *s, const char *accept);
