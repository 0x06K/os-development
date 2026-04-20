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



// table of kernel functions, index = syscall number
typedef int (*syscall_fn)(uint32_t, uint32_t, uint32_t);

static syscall_fn syscall_table[] = {
    [0] = (syscall_fn)kprintf,
    [1] = (syscall_fn)kgets,
};

void syscall_handler(registers_t *regs) {
    uint32_t num = regs->eax;  // get syscall number

    // bounds check + null check
    if (num < sizeof(syscall_table)/sizeof(syscall_table[0]) && syscall_table[num]) {
        // call the function, pass EBX, ECX, EDX as args
        regs->eax = syscall_table[num](regs->ebx, regs->ecx, regs->edx);
    } else {
        regs->eax = -1;  // unknown syscall
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

extern volatile uint8_t status;
__attribute__((interrupt))
void irq14_handler(interrupt_frame* frame) {
    outb(0x20, 0x20);   // EOI to master PIC
    outb(0xA0, 0x20);   // EOI to slave PIC    
    (void)frame;
    status = 1;
}