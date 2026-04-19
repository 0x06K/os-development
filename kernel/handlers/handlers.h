#ifndef HANDLERS_H
#define HANDLERS_H

#include <stdint.h>

typedef struct _interrupt_frame
{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
}interrupt_frame;

// keyboard handler
__attribute__((interrupt))
void keyboard_handler(interrupt_frame *frame);

// timer handler
__attribute__((interrupt))
void timer_handler(interrupt_frame *frame);

// syscall handler
typedef struct {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} registers_t;


void syscall_handler(registers_t regs);
__attribute__((naked)) void syscall_stub(void);


__attribute__((interrupt))
void irq14_handler(interrupt_frame* frame);


#endif