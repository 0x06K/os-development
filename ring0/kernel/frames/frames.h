#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include <stdbool.h>


// Initialize the frame allocator
void frame_init();

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