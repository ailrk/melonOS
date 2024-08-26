#pragma once

#include <stdint.h>

int   memcmp(const void*, const void*, uint32_t);
void *memmove(void*, const void*, uint32_t);
void  memcpy(void *, const void*, uint32_t);
void *memset(void*, int, uint32_t);
int   strlen(const char*);
int   strncmp(const char*, const char*, uint32_t);
char *strncpy(char*, const char*, uint32_t);
char *strrev(char *s);
