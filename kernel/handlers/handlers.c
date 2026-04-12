#pragma GCC target("general-regs-only")
#include <vga/vga.h>
#include <keyboard/keyboard.h>
#include "handlers.h"

extern void keyboard_push(uint8_t scancode);

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


__attribute__((interrupt)) 
void keyboard_handler(interrupt_frame *frame)
{
    (void)frame;
    uint8_t sc = inb(0x60);
    keyboard_push(sc);
    outb(0x20, 0x20);
}

__attribute__((interrupt))
void timer_handler(interrupt_frame *frame)
{
    (void)frame;
    outb(0x20, 0x20);
}



void syscall_handler(registers_t regs) {
    switch (regs.eax) {
        case 0:
            for(;;) asm volatile("hlt");
            break;
        case 1:
            kprintf((const char*)regs.ebx);
            break;
    }
}
__attribute__((naked))
void syscall_stub(void) {
    asm volatile(
        "pusha              \n"
        "call syscall_handler\n"
        "popa               \n"
        "iret               \n"
    );
}



