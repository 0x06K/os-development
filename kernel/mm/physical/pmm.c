#include<kernel/mm/physical/pmm.h>

void pmm_init(uint32_t mem_size) {
    frame_init(mem_size);
}
void* pmm_alloc(uint32_t num_pages) {

    switch(num_pages){
    case 0:     // Make sure we’re not asking for 0 pages
        return 0;

    case 1:
        return (void*)frame_alloc(num_pages);

    default:
        uint32_t first_frame = 0;
        uint32_t count = 0;

        // Try to find a contiguous block of free frames
        for (uint32_t i = 0; i < total_frames; i++) {
            if (frame_is_free(i)) {
                // found a free frame
                if (count == 0)
                    first_frame = i;
                count++;

                // found enough frames in a row
                if (count == num_pages) {
                    // mark them all used
                    for (uint32_t j = 0; j < num_pages; j++)
                        frame_set_used(first_frame + j);

                    return (void*)(first_frame * FRAME_SIZE);
                }
            } else {
                // sequence broke — reset
                count = 0;
            }
        }
    }

    // no block found — out of memory
    return 0;
}


void pmm_free(void* addr, uint32_t num_pages) {
    if (!addr || num_pages == 0)
        return;

    // Get the starting frame index from the physical address
    uint32_t frame = (uint32_t)addr / FRAME_SIZE;

    // Loop through all pages to free them
    for (uint32_t i = 0; i < num_pages; i++) {
        frame_free(frame + i);
    }
}

uint32_t pmm_get_used_mem(void) {
    return frame_get_used() * FRAME_SIZE;   // bytes of used physical memory
}

uint32_t pmm_get_free_mem(void) {
    return (frame_get_total() - frame_get_used()) * FRAME_SIZE;  // bytes of free memory
}

