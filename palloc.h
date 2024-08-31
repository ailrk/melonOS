#pragma once


void palloc_init(void *vstart, void *vend);
char *palloc();
void pfree(char *);
