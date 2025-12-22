#ifndef KHEAP_H
#define KHEAP_H

#define KHEAP_START  0xC1000000  // example virtual address
#define KHEAP_INITIAL_SIZE 0x100000  // 1 MB initial heap
#define KHEAP_MAX     0xC2000000  // max heap end address
#define PAGE_SIZE 4096

void  kheap_init(uint32_t start_addr, uint32_t end_addr);
void* kmalloc(uint32_t size);
void* kmalloc_a(uint32_t size);               // page aligned
void* kmalloc_p(uint32_t size, uint32_t* phys);   // returns physical address
void* kmalloc_ap(uint32_t size, uint32_t* phys);  // aligned + physical
void  kfree(void* ptr);

#endif
