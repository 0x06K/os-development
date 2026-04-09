#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdint.h>

struct interrupt_frame
{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
};


__attribute__((interrupt))
void keyboard_handler(struct interrupt_frame *frame);


__attribute__((interrupt))
void timer_handler(struct interrupt_frame *frame);

#endif