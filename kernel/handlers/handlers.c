#pragma GCC target("general-regs-only")

#include "handlers.h"

volatile int scancode;

uint8_t inb(uint16_t port)
{
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// make sure you are using the right calling convention when writing isr.
__attribute__((interrupt))
void keyboard_handler(struct interrupt_frame *frame)
{
    scancode = inb(0x60);
    outb(0x20, 0x20);
}

__attribute__((interrupt))
void timer_handler(struct interrupt_frame *frame)
{
    outb(0x20, 0x20);
}