#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include <stdbool.h>

#define FRAME_SIZE 4096  // 4KB per physical frame

// ======================================================
// Low Level Implementation of Frame management
// ======================================================

#define FRAME_SIZE         4096U          // 4 KB per frame
#define MEMORY_BASE        0x00100000U    // 1 MB, typical usable memory start
#define BITS_PER_BYTE      8
#define BITMAP_START       0x00200000U

// Initialize the frame allocator
void frame_init(uint32_t mem_size);

// Mark a specific frame as used/free
void frame_set_used(uint32_t frame);
void frame_set_free(uint32_t frame);

// Query frame state
bool frame_is_free(uint32_t frame);

// Find and allocate/free frames
uint32_t frame_find_free(void);
uint32_t frame_alloc(void);
void frame_free(uint32_t addr);

#endif