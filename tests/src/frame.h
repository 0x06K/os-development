#ifndef FRAME_H
#define FRAME_H

#include <stdbool.h>
#include <stdint.h>

void frame_init();

uint32_t find_free_frame(void);

bool frame_is_free(uint32_t);
void* frame_alloc();
void* nframe_alloc(uint32_t);
void* xframe_alloc(uint32_t, uint32_t);
void frame_set_free(uint32_t);
void xframe_set_free(uint32_t, uint32_t);
void nframe_set_free(uint32_t, uint32_t);


#endif
