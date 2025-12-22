#include <kernel/mm/virtual/page_table.h>
#include <kernel/mm/physical/pmm.h>


#define KERNEL_BASE 0xC0000000
#define phys_to_virt(p) ((void *)((p) + KERNEL_BASE))

uint32_t *kernel_page_directory = 0;

void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint32_t dir_idx   = page_dir_index(virt_addr);
    uint32_t table_idx = page_table_index(virt_addr);
    uint32_t *pd       = kernel_page_directory;
    if (!pd) return;

    /* Allocate a new page table if missing */
    if (!(pd[dir_idx] & PAGE_PRESENT)) {
        uint32_t phys_table = pmm_alloc();
        if (!phys_table) return;

        uint32_t *new_table = (uint32_t *)phys_to_virt(phys_table);
        for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++)
            new_table[i] = 0;

        pd[dir_idx] = phys_table | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    uint32_t *pt = (uint32_t *)phys_to_virt(pd[dir_idx] & 0xFFFFF000);
    pt[table_idx] = (phys_addr & 0xFFFFF000) | (flags & 0xFFF);

    asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
}

void page_unmap(uint32_t virt_addr) {
    uint32_t dir_idx   = page_dir_index(virt_addr);
    uint32_t table_idx = page_table_index(virt_addr);
    uint32_t *pd       = kernel_page_directory;
    if (!pd) return;

    if (!(pd[dir_idx] & PAGE_PRESENT)) return;

    uint32_t *pt = (uint32_t *)phys_to_virt(pd[dir_idx] & 0xFFFFF000);
    pt[table_idx] = 0;

    asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
}

uint32_t page_get_phys(uint32_t virt_addr) {
    uint32_t dir_idx   = page_dir_index(virt_addr);
    uint32_t table_idx = page_table_index(virt_addr);
    uint32_t *pd       = kernel_page_directory;
    if (!pd) return 0;

    if (!(pd[dir_idx] & PAGE_PRESENT)) return 0;

    uint32_t *pt = (uint32_t *)phys_to_virt(pd[dir_idx] & 0xFFFFF000);
    uint32_t entry = pt[table_idx];
    if (!(entry & PAGE_PRESENT)) return 0;

    return entry & 0xFFFFF000;
}
