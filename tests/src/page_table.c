#include <stdint.h>
#include "page_table.h"
#include "frame.h"
#include "config.h"
#include "vga.h"
uint32_t* kernel_page_directory = NULL;



void pd_init() {
    // Allocate and clear the Page Directory
    kernel_page_directory = (uint32_t*)frame_alloc();
    for (uint32_t page = 0; page < 1024; page++)
        kernel_page_directory[page] = 0;
    vga_writestring("page directory zeroed out.\n");

    // Identity map initial 4MB (0x0 → 0x3FFFFF)
    for (uint32_t table = 0; table < 4; table++) {
        for (uint32_t page_table_entry = 0; page_table_entry < 1024; page_table_entry++) {
            uint32_t page_index = (table * 1024) + page_table_entry;
            uint32_t phys = page_index * PAGE_SIZE;
            uint32_t virt = phys;
            page_map(virt, phys, PAGE_PRESENT | PAGE_RW);
        }
    }
    vga_writestring("Initial 4MB mapped out.\n");

    // Mapping Kernel at virtual base: 0xC0010000
    // Physical range: 0x10000 - > 0x2F8000
    // Virtual range: 0xC0010000 -> 0xC02F8000
    for (uint32_t page_table_entry = 0; page_table_entry < 745; page_table_entry++) {
        uint32_t phys = KERNEL_PHYS_BASE + (page_table_entry * PAGE_SIZE);
        uint32_t virt = KERNEL_VIRTUAL_BASE + (page_table_entry * PAGE_SIZE);
        page_map(virt, phys, PAGE_PRESENT | PAGE_RW);
    }
    vga_writestring("Kernel mapped at virtual address.\n");

    // Mapping Bitmap at virtual base: 0xC02F8000
    // physical range: 0x2F8000 -> 0x300000
    // virtual range: 0xC02F8000 -> 0xC0300000
    for (uint32_t page_table_entry = 0; page_table_entry < 8; page_table_entry++) {
        uint32_t phys = BITMAP_PHYS_BASE + (page_table_entry * PAGE_SIZE);
        uint32_t virt = BITMAP_VIRTUAL_BASE + (page_table_entry * PAGE_SIZE);
        page_map(virt, phys, PAGE_PRESENT | PAGE_RW);
    }
    vga_writestring("Bitmap done.\n");

    // Mapping Heap at virtual base: 0xC0300000
    // physical range: 0x300000 -> 0xF00000
    // virtual range: 0xC0300000 -> 0xC0F00000
    for (uint32_t i = 0; i < ((0xF00000 - 0x300000) / PAGE_SIZE); i++) {
        uint32_t phys = KHEAP_PHYS_BASE + i * PAGE_SIZE;
        uint32_t virt = KHEAP_VIRTUAL_BASE + i * PAGE_SIZE;
        page_map(virt, phys, PAGE_PRESENT | PAGE_RW);
    }
    vga_writestring("Heap done.\n");

    // Mapping Stack at virtual base: 0xC0F00000
    // physical range: 0xF00000 -> 0x1000000
    // virtual range: 0xC0F00000 -> 0xC1000000
    for (uint32_t page_table_entry = 0; page_table_entry < 256; page_table_entry++) {
        uint32_t phys = KERNEL_STACK_BOTTOM + (page_table_entry * PAGE_SIZE);
        uint32_t virt = KERNEL_STACK_VIRTUAL_BASE + (page_table_entry * PAGE_SIZE);
        page_map(virt, phys, PAGE_PRESENT | PAGE_RW);
    }
    vga_writestring("Stack done.\n");

    // load page directory
    asm volatile("mov %0, %%cr3" :: "r"(kernel_page_directory));

    // enable paging
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set the PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint32_t dir_idx = page_dir_index(virt_addr);
    uint32_t table_idx = page_table_index(virt_addr);

    uint32_t pde = kernel_page_directory[dir_idx];
    uint32_t* page_table;

    // Check if page table exists
    if (pde & PAGE_PRESENT) {
        // Get existing page table (mask out flags)
        page_table = (uint32_t*)(pde & 0xFFFFF000);
    } else {
        // Allocate and clear new page table
        page_table = (uint32_t*)frame_alloc();
        for (int i = 0; i < 1024; i++) page_table[i] = 0;

        // Link page table in directory
        kernel_page_directory[dir_idx] =
            ((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW;
    }

    // Map the entry
    page_table[table_idx] = (phys_addr & 0xFFFFF000) | (flags & 0xFFF);

    asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");

}

bool page_is_mapped(uint32_t virt) {
    uint32_t dir_idx   = page_dir_index(virt);
    uint32_t table_idx = page_table_index(virt);

    uint32_t pde = kernel_page_directory[dir_idx];

    // PDE must be present
    if (!(pde & PAGE_PRESENT))
        return false;

    uint32_t *page_table = (uint32_t*)(pde & 0xFFFFF000);

    uint32_t pte = page_table[table_idx];

    // PTE present = page is mapped
    return (pte & PAGE_PRESENT) != 0;
}
void npage_map(uint32_t virt, uint32_t phys, uint32_t flags, uint32_t num_of_pages){
  for(uint32_t page = 0; page < num_of_pages; page++) {
    page_map(virt, phys, flags);
  }
}

void* npage_is_mapped(uint32_t num_pages, uint32_t search_start, uint32_t search_end)
{
    uint32_t consecutive = 0;
    uint32_t start_addr  = 0;

    // scan 4KB at a time
    for (uint32_t addr = search_start; addr < search_end; addr += PAGE_SIZE)
    {
        if (!page_is_mapped(addr))  // page is free
        {
            if (consecutive == 0)
                start_addr = addr;

            consecutive++;

            if (consecutive == num_pages)
            {
                // We found a block: return starting virtual address
                return (void*)start_addr;
            }
        }
        else
        {
            consecutive = 0;
        }
    }

    return NULL; // no space found
}

int page_unmap(uint32_t virt_addr) {
    uint32_t dir_idx = page_dir_index(virt_addr);
    uint32_t table_idx = page_table_index(virt_addr);
    uint32_t pde = kernel_page_directory[dir_idx];

    if(!(pde & PAGE_PRESENT)) return -1; // table doesn't exist

    uint32_t* page_table = (uint32_t*)(pde & 0xFFFFF000);
    page_table[table_idx] = 0;

    asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");

    return 0;
}
