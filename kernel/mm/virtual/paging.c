#include <kernel/mm/paging.h>
#include <kernel/mm/virtual/page_table.h>
#include <kernel/mm/physical/pmm.h>


/* kernel_page_directory declared in page_table.c (weak linkage) */
extern uint32_t *kernel_page_directory;

/* Helper to load CR3 with a physical address */
static inline void load_cr3(uint32_t phys) {
    asm volatile("mov %0, %%cr3" :: "r"(phys));
}

/* Helper to enable paging (set CR0.PG) */
static inline void enable_paging(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; /* set paging bit */
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

/* Create PD, identity-map kernel region, enable paging */
void paging_init(uint32_t kernel_phys_start, uint32_t kernel_phys_end) {
    /* allocate a page for the page directory */
    kernel_page_directory = (uint32_t *)pmm_alloc();
    if (!kernel_page_directory) return;

    /* Zero directory */
    for (uint32_t i = 0; i < PAGE_DIR_ENTRIES; ++i)
        kernel_page_directory[i] = 0;

    /* Identity-map kernel range: map each 4KB page phys->virt (virt == phys) */
    uint32_t start = kernel_phys_start & 0xFFFFF000;
    uint32_t end   = (kernel_phys_end + PAGE_SIZE - 1) & 0xFFFFF000;

    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        /* use page_map which will allocate page tables as needed */
        page_map(addr, addr, PAGE_PRESENT | PAGE_RW);
    }

    /* Load CR3 with physical addr of page directory (assume identity mapping of pd) */
    load_cr3((uint32_t)kernel_page_directory & 0xFFFFF000);

    /* enable paging */
    enable_paging();
}

/* Switch CR3 to a physical page directory address */
void paging_switch_directory(uint32_t phys_pd) {
    load_cr3(phys_pd & 0xFFFFF000);
}

/* Return current CR3 (physical) */
uint32_t paging_get_current_cr3(void) {
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

/* Very tiny page fault handler stub.
 * err_code and fault_addr provided by ISR assembly wrapper.
 * Replace with proper logging / panic.
 */
void paging_handle_fault(uint32_t err_code, uint32_t fault_addr) {
    (void)err_code;
    (void)fault_addr;
    /* hang for now */
    for (;;) asm volatile("hlt");
}
