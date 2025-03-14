#pragma once

void  palloc_init (void *vstart, void *vend);
char *palloc ();
void  pfree (char *);
typedef struct Run { struct Run *next; } Run;


#if DEBUG
void pmem_report();
#endif
