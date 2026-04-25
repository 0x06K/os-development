#include "loader.h"

void jump_usermode(uint32_t entry, uint32_t user_stack) {
    __asm__ volatile (
        "cli                    \n"  // disable interrupts

        // set segment registers to user data segment (ring 3)
        // 0x23 = GDT user data selector | RPL 3
        "mov $0x23, %%ax        \n"
        "mov %%ax, %%ds         \n"
        "mov %%ax, %%es         \n"
        "mov %%ax, %%fs         \n"
        "mov %%ax, %%gs         \n"

        // build fake iret frame on stack
        // iret pops: EIP, CS, EFLAGS, ESP, SS
        "push $0x23             \n"  // SS  — user stack segment
        "push %1                \n"  // ESP — user stack pointer
        "pushf                  \n"  // EFLAGS
        "pop %%eax              \n"
        "or $0x200, %%eax       \n"  // set IF flag (enable interrupts in ring 3)
        "push %%eax             \n"
        "push $0x1B             \n"  // CS  — user code segment (0x1B = ring 3)
        "push %0                \n"  // EIP — entry point of loaded program

        "iret                   \n"  // jump to ring 3
        :
        : "r"(entry), "r"(user_stack)
        : "eax", "memory"
    );
}
