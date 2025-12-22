#include <kernel/mm/vmm.h>
#include <kernel/mm/virtual/page_table.h>
#include <kernel/mm/physical/pmm.h>



/* thin wrappers */
void vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    page_map(virt_addr, phys_addr, flags);
}

void vmm_unmap_page(uint32_t virt_addr) {
    page_unmap(virt_addr);
}

int vmm_alloc_npage(uint32_t virt_addr, uint32_t num_of_pages, uint32_t flags) {
    uint32_t *first_frame = (uint32_t*)pmm_alloc(num_of_pages);
    if (first_frame == 0)
        return -1;

    uint32_t phys_addr = (uint32_t)first_frame;

    for (uint32_t i = 0; i < num_of_pages; i++) {
        page_map(virt_addr + i * PAGE_SIZE, phys_addr + i * PAGE_SIZE, flags);
    }

    return 0;
}


/* allocate a frame and map it at virt */
int vmm_alloc_page(uint32_t virt_addr, uint32_t flags) {
    void *frame = pmm_alloc();
    if (!frame) return -1;
    vmm_map_page(virt_addr, (uint32_t)frame, flags | PAGE_PRESENT);
    return 0;
}

/* free a mapped page (unmap + free phys frame) */
int vmm_free_page(uint32_t virt_addr) {
    uint32_t phys = vmm_get_phys(virt_addr);
    if (!phys) return -1;
    vmm_unmap_page(virt_addr);
    pmm_free((void *)phys);
    return 0;
}

uint32_t vmm_get_phys(uint32_t virt_addr) {
    return page_get_phys(virt_addr);
}
