#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <stdint.h>

/* Page flags */
#define PAGE_PRESENT 0x001
#define PAGE_RW      0x002
#define PAGE_USER    0x004
#define PAGE_WRITE   PAGE_RW

/* Page table/directory entries */
#define PAGE_SIZE          0x1000
#define PAGE_DIR_ENTRIES   1024
#define PAGE_TABLE_ENTRIES 1024

/* Kernel page directory pointer (set in paging init) */
extern uint32_t *kernel_page_directory;

/* Helpers to get indexes */
static inline uint32_t page_dir_index(uint32_t addr) {
    return (addr >> 22) & 0x3FF;
}

static inline uint32_t page_table_index(uint32_t addr) {
    return (addr >> 12) & 0x3FF;
}

/* Low-level page operations */
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_unmap(uint32_t virt_addr);
uint32_t page_get_phys(uint32_t virt_addr);

#endif /* PAGE_TABLE_H */
