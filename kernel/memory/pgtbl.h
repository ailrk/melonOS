#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "mmu.h"
#include "vmem.h"


PDE       *get_pde(PageDir pgdir, const void *vaddr);
PageTable  get_pt(PageDir pgdir, const void *vaddr);
PageTable  get_pt1(PDE *pde);
PTE       *get_pte(PageDir pgdir, const void *vaddr);
PTE       *get_pte1(PDE *pde, const void *vaddr);

PTE       *walk(PageDir pgdir, const void *vaddr);
bool       map_pages(PageDir pgdir, const VMap *k);
void       unmap_pages(PageDir pgdir, uintptr_t vstart, size_t n, bool free);
void       set_pte_flag(PageDir pgdir, char *vaddr, unsigned flag);
void       clear_pte_flag(PageDir pgdir, char *vaddr, unsigned flag);
