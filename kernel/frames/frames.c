#include <stddef.h>
#include "frames.h"

#define FRAME_SIZE         4096U
#define BITS_PER_BYTE      8
#define BITMAP_START       0x00200000U
#define TOTAL_MEMORY       0x100000000ULL
#define TOTAL_FRAMES       (TOTAL_MEMORY / FRAME_SIZE)
#define BITMAP_SIZE        (TOTAL_FRAMES / BITS_PER_BYTE)

static uint8_t  *frame_bitmap = NULL;
static uint32_t  used_frames  = 0;

void frame_init(void) {
    frame_bitmap = (uint8_t*)BITMAP_START;

    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
        frame_bitmap[i] = 0xFF;

    used_frames = TOTAL_FRAMES;

    uint32_t first_free_frame = 0x500000 / FRAME_SIZE;
    for (uint32_t i = first_free_frame; i < TOTAL_FRAMES; i++) {
        frame_bitmap[i / BITS_PER_BYTE] &= ~(1 << (i % BITS_PER_BYTE));
        used_frames--;
    }
}

void frame_set_used(uint32_t frame) {
    frame_bitmap[frame / BITS_PER_BYTE] |= (1 << (frame % BITS_PER_BYTE));
    used_frames++;
}

void frame_set_free(uint32_t frame) {
    frame_bitmap[frame / BITS_PER_BYTE] &= ~(1 << (frame % BITS_PER_BYTE));
    used_frames--;
}

bool frame_is_free(uint32_t frame) {
    return !(frame_bitmap[frame / BITS_PER_BYTE] & (1 << (frame % BITS_PER_BYTE)));
}

uint32_t frame_alloc(void) {
    for (uint32_t byte = 0; byte < BITMAP_SIZE; byte++) {
        if (frame_bitmap[byte] == 0xFF)
            continue;
        for (uint8_t bit = 0; bit < BITS_PER_BYTE; bit++) {
            if (!(frame_bitmap[byte] & (1 << bit))) {
                uint32_t frame = byte * BITS_PER_BYTE + bit;
                frame_set_used(frame);
                return frame * FRAME_SIZE;
            }
        }
    }
    return 0;
}

void frame_free(uint32_t addr) {
    frame_set_free(addr / FRAME_SIZE);
}