// paging.h
#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// page directory and table flags
#define PAGE_PRESENT    0x1
#define PAGE_RW         0x2
#define PAGE_USER       0x4

void paging_init(void);
void map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags);
void unmap_page(uint32_t vaddr);


#endif