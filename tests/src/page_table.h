#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <stdint.h>
#include <stddef.h>
#include "config.h"


/* Page table/directory entries */
#define PAGE_SIZE          0x1000
#define PAGE_DIR_ENTRIES   1024
#define PAGE_TABLE_ENTRIES 1024


/* Kernel page directory pointer (set in paging init) */
extern uint32_t *kernel_page_directory;

/* Helpers to get indexes */
inline uint32_t page_dir_index(uint32_t addr) {
    return (addr >> 22) & 0x3FF;
}

inline uint32_t page_table_index(uint32_t addr) {
    return (addr >> 12) & 0x3FF;
}
inline uint32_t page_get_phys(uint32_t virt) {
    return ((KERNEL_VIRTUAL_BASE - KERNEL_START) + (kernel_page_directory[page_dir_index(virt)] & 0xFFFFF000));
}

void pd_init();
void page_map(uint32_t, uint32_t, uint32_t);
int page_unmap(uint32_t);
void* npage_is_mapped(uint32_t, uint32_t, uint32_t);
void npage_map(uint32_t, uint32_t, uint32_t, uint32_t);


#endif /* PAGE_TABLE_H */
