#ifndef PMM_H
#define PMM_H

// ======================================================
// Physical Memory Manager (PMM)
// High-level wrapper over frame allocator
// ======================================================

#include <stdint.h>
#include <stdbool.h>
#include "frame.h"

// variables in frame.c
extern uint32_t total_frames;
extern uint32_t used_frames;

// Initialize PMM with total physical memory size
void pmm_init(uint32_t mem_size);

// Allocate a contiguous block of physical pages
// Returns physical address of the first page
void* pmm_alloc(uint32_t num_pages);
// allocates 1 page
inline void* pmm_alloc() { return pmm_alloc(1); }
// Free a contiguous block of physical pages
void pmm_free(void* addr, uint32_t num_pages);

// Get the amount of used / free memory (optional, useful for debugging)
uint32_t pmm_get_used_mem(void);
uint32_t pmm_get_free_mem(void);

uint32_t frame_get_total(void);
uint32_t frame_get_used(void);

#endif // PMM_H
