// paging.c
#include "paging.h"

// page directory at 1MB
uint32_t *page_directory = (uint32_t*)0x00100000;

// page tables start at 1MB + 4KB
uint32_t *page_tables    = (uint32_t*)0x00101000;

void paging_init(void) {

    // zero out page directory
    for (int i = 0; i < 1024; i++)
        page_directory[i] = 0;

    // ─────────────────────────────────────────
    // map first 1GB as kernel (U/S=0)
    // 0x00000000 - 0x3FFFFFFF
    // 256 page tables (entries 0-255 in PD)
    // ─────────────────────────────────────────
    for (int pd_i = 0; pd_i < 256; pd_i++) {
        uint32_t *pt = page_tables + (pd_i * 1024);

        // fill each page table with 1024 entries
        for (int pt_i = 0; pt_i < 1024; pt_i++) {
            uint32_t physical = (pd_i * 1024 + pt_i) * 0x1000;
            pt[pt_i] = physical | PAGE_PRESENT | PAGE_RW;  // U/S=0 kernel only
        }

        // point page directory entry to this page table
        page_directory[pd_i] = (uint32_t)pt | PAGE_PRESENT | PAGE_RW;  // U/S=0
    }

    // ─────────────────────────────────────────
    // map next 3GB as user (U/S=1)
    // 0x40000000 - 0xBFFFFFFF
    // 768 page tables (entries 256-1023 in PD)
    // ─────────────────────────────────────────
    for (int pd_i = 256; pd_i < 1024; pd_i++) {
        uint32_t *pt = page_tables + (pd_i * 1024);

        for (int pt_i = 0; pt_i < 1024; pt_i++) {
            uint32_t physical = (pd_i * 1024 + pt_i) * 0x1000;
            pt[pt_i] = physical | PAGE_PRESENT | PAGE_RW | PAGE_USER;  // U/S=1
        }

        page_directory[pd_i] = (uint32_t)pt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    // load page directory into CR3
    asm volatile("mov %0, %%cr3" : : "r"(page_directory));

    // enable paging by setting bit 31 of CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}
void map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    uint32_t pd_i = vaddr >> 22;
    uint32_t pt_i = (vaddr >> 12) & 0x3FF;

    uint32_t *pt = (uint32_t*)(page_directory[pd_i] & ~0xFFF);
    pt[pt_i] = paddr | flags;

    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

void unmap_page(uint32_t vaddr) {
    uint32_t pd_i = vaddr >> 22;
    uint32_t pt_i = (vaddr >> 12) & 0x3FF;

    uint32_t *pt = (uint32_t*)(page_directory[pd_i] & ~0xFFF);
    pt[pt_i] = 0;

    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}