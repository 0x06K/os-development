#include <kernel/mm/physical/frame.h>

// Global vars to track frames
static uint8_t* frame_bitmap = NULL;
static uint32_t total_frames = 0;
static uint32_t used_frames = 0;

void frame_init(uint32_t mem_size) {
    // Calculate total frames
    total_frames = mem_size / FRAME_SIZE;

    // Calculate bitmap size (1 bit per frame)
    uint32_t bitmap_size = total_frames / BITS_PER_BYTE;
    if (total_frames % BITS_PER_BYTE) bitmap_size++;

    // Place the bitmap somewhere safe (right after kernel or a fixed base)
    frame_bitmap = (uint8_t*)BITMAP_START;

    // Mark all frames as used initially
    for (uint32_t i = 0; i < bitmap_size; i++) {
        frame_bitmap[i] = 0xFF;
    }
    used_frames = total_frames;

    // Mark usable memory as free (for now, assume all after MEMORY_BASE is free)
    uint32_t first_free_frame = MEMORY_BASE / FRAME_SIZE;
    for (uint32_t i = first_free_frame; i < total_frames; i++) {
        uint32_t byte = i / BITS_PER_BYTE;
        uint8_t bit  = i % BITS_PER_BYTE;
        frame_bitmap[byte] &= ~(1 << bit);
        used_frames--;
    }

    // Reserve kernel + bitmap area again to avoid reuse
    uint32_t bitmap_end_frame = (BITMAP_START + bitmap_size) / FRAME_SIZE;
    for (uint32_t i = 0; i < bitmap_end_frame; i++) {
        uint32_t byte = i / BITS_PER_BYTE;
        uint8_t bit  = i % BITS_PER_BYTE;
        frame_bitmap[byte] |= (1 << bit);
        used_frames++;
    }
}

void frame_set_used(uint32_t frame) {
    uint32_t byte = frame / BITS_PER_BYTE;
    uint8_t  bit  = frame % BITS_PER_BYTE;
    frame_bitmap[byte] |= (1 << bit);  // mark frame as used
    used_frames++;
}

void frame_set_free(uint32_t frame) {
    uint32_t byte = frame / BITS_PER_BYTE;
    uint8_t  bit  = frame % BITS_PER_BYTE;
    frame_bitmap[byte] &= ~(1 << bit); // mark frame as free
    used_frames--;
}

bool frame_is_free(uint32_t frame){
    uint32_t byte = frame / BITS_PER_BYTE;
    uint8_t  bit  = frame % BITS_PER_BYTE;
    return !(frame_bitmap[byte] & (1 << bit));
}

uint32_t frame_find_free(void) {
    // Loop through each byte in the bitmap
    for (uint32_t byte = 0; byte < bitmap_size; byte++) {
        if (frame_bitmap[byte] != 0xFF) { // 0xFF = all 8 frames used
            // Check each bit in this byte
            for (uint8_t bit = 0; bit < BITS_PER_BYTE; bit++) {
                if (!(frame_bitmap[byte] & (1 << bit))) {
                    // Found a free frame!
                    return (byte * BITS_PER_BYTE) + bit;
                }
            }
        }
    }
    return 0xFFFFFFFF; // No free frame found
}

uint32_t frame_alloc(void) {
    uint32_t frame = frame_find_free();
    if (frame == 0xFFFFFFFF) {
        return 0; // Out of memory
    }

    frame_set_used(frame);
    return frame * FRAME_SIZE; // Return physical address
}

void frame_free(uint32_t addr) {
    uint32_t frame = addr / FRAME_SIZE;
    frame_set_free(frame);
}

uint32_t frame_get_total(void) { return total_frames; }
uint32_t frame_get_used(void)  { return used_frames; }