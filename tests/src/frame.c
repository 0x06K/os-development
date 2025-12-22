#include "frame.h"
#include "config.h"

#include <stdint.h>
#include <stddef.h>




static uint32_t* bitmap = (uint32_t*)BITMAP_PHYS_BASE;

void frame_init() {

    /* Mark initial 16MB as allocated
        1MB-2MB Kernel code/data
        2MB-3MB Kernel Stack
        3MB-16MB Kernel Heap
    */
    for (uint32_t page = 0; page < 4096; page++) bitmap[page] = 0xFFFFFFFF;


    // Mark remaining frames as free
    for (uint32_t i = 512; i < BITMAP_SIZE; i++) bitmap[i] = 0x0;

}

uint32_t find_free_frame() {
    for (uint32_t i = 0; i < BITMAP_SIZE; i++) {

        // if this 32-bit chunk has at least ONE zero bit (means free frame)
        if (bitmap[i] != 0xFFFFFFFF) {

            for (uint8_t bit = 0; bit < 32; bit++) {

                // check if the bit is free (0 = free)
                if ((bitmap[i] & (1u << bit)) == 0) {

                    // return absolute frame number
                    return i * 32 + bit;
                }
            }
        }
    }

    return 0; // bruh moment: no free frames
}

void* frame_alloc() {
    uint32_t frame_num = find_free_frame();

    if (frame_num == 0)
        return NULL;

    uint32_t idx = frame_num / 32;
    uint8_t bit  = frame_num % 32;

    bitmap[idx] |= (1u << bit);

    return (void*)(frame_num * 4096);
}

void* nframe_alloc(uint32_t num_of_frames)
{
    uint32_t total_bits = TOTAL_FRAMES;
    uint32_t consecutive = 0;
    uint32_t start_frame = 0;

    for (uint32_t frame = 0; frame < total_bits; frame++) {

        uint32_t idx = frame / 32;
        uint32_t bit = frame % 32;

        // check if free
        if ((bitmap[idx] & (1u << bit)) == 0) {

            if (consecutive == 0)
                start_frame = frame;

            consecutive++;

            if (consecutive == num_of_frames) {
                // mark them used
                for (uint32_t f = start_frame; f < start_frame + num_of_frames; f++) {
                    uint32_t i2 = f / 32;
                    uint32_t b2 = f % 32;
                    bitmap[i2] |= (1u << b2);
                }
                return (void*)(start_frame * 4096);
            }
        }
        else {
            consecutive = 0;
        }
    }
    return NULL;
}

bool frame_is_free(uint32_t frame_count) {
    uint32_t idx = frame_count / 32;
    uint32_t bit = frame_count % 32;
    return ((bitmap[idx] & (1 << bit)) == 0);
}

void* xframe_alloc(uint32_t phys_addr, uint32_t num_of_frame) {
    if (phys_addr % FRAME_SIZE != 0) return NULL;

    uint32_t frame_number = phys_addr / FRAME_SIZE;
    uint32_t required_frame_number = frame_number + num_of_frame;
    // check if every frame is free
    for (uint32_t frame = frame_number; frame < required_frame_number; frame++) {
        if (!frame_is_free(frame))
            return NULL;
    }

    // mark frames
    for (uint32_t frame = frame_number; frame < required_frame_number; frame++) {
        uint32_t i = frame / 32;
        uint32_t b = frame % 32;
        bitmap[i] |= (1u << b);
    }

    return (void*)(frame_number * FRAME_SIZE);
}

void frame_set_free(uint32_t phys){
  if(phys % FRAME_SIZE != 0)
    phys = phys & ~(FRAME_SIZE - 1);
  uint32_t frame = phys / FRAME_SIZE;
  uint32_t idx = frame / 32;
  uint32_t bit = frame % 32;
  bitmap[idx] &= ~(1 << bit);
}

void xframe_set_free(uint32_t start_addr, uint32_t end_addr){
  if(start_addr % FRAME_SIZE != 0)
    start_addr = start_addr & ~(FRAME_SIZE - 1);
  if(end_addr % FRAME_SIZE != 0)
    end_addr = end_addr & ~(FRAME_SIZE - 1);
  for(uint32_t start = start_addr; start <= end_addr; start+=FRAME_SIZE){
      frame_set_free(start);
    }
}
void nframe_set_free(uint32_t start_addr, uint32_t num_of_frames){
  if(start_addr % FRAME_SIZE != 0)
    start_addr = start_addr & ~(FRAME_SIZE - 1);
  for(uint32_t frame = 0; frame < num_of_frames; frame++){
    frame_set_free(start_addr);
    start_addr += FRAME_SIZE;
  }
}
