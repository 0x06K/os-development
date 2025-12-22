#ifndef KERNEL_MM_VMM_H
#define KERNEL_MM_VMM_H

#include <stdint.h>

/* Initialize VMM layer (must be called after paging_init) */
inline void vmm_init(void) { paging_init(); }

/* Map/unmap helpers that wrap page_table functions (same signatures) */
int vmm_alloc_npage(uint32_t virt_addr, uint32_t num_of_pages, uint32_t flags)
void vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void vmm_unmap_page(uint32_t virt_addr);

/* Allocate a physical frame and map it at virt (flags include rw/user) */
int vmm_alloc_page(uint32_t virt_addr, uint32_t flags); /* returns 0 on success, -1 on fail */

/* Free a mapped page: unmap + free phys frame */
int vmm_free_page(uint32_t virt_addr); /* returns 0 on success, -1 on fail */

/* Return physical mapped addr or 0 */
uint32_t vmm_get_phys(uint32_t virt_addr);

#endif /* KERNEL_MM_VMM_H */
