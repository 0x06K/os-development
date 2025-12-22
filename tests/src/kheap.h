#ifndef KHEAP_H
#define KHEAP_H

#include "config.h"
#include "page_table.h"
#include "frame.h"
#include <stdint.h>



// #define PAGE_SIZE 4096

void  kheap_init(uint32_t, uint32_t);
void* kmalloc(uint32_t);
void* kmalloc_a(uint32_t);               // page aligned
void* kmalloc_p(uint32_t);   // returns physical address
void* kmalloc_ap(uint32_t);  // aligned + physical
void  kfree(void* ptr);

#endif
