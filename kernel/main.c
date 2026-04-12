#include <vga/vga.h>
#include <idt/idt.h>
#include <gdt/gdt.h>
#include <keyboard/keyboard.h>
#include <tss/tss.h>

extern void user_program(void);

#define USER_EIP (uint32_t)user_program
#define USER_ESP 0xBFFFFFF0

__attribute__((naked))
void jump_to_usermode(void) {
    asm volatile(
        "mov $0x23, %%ax    \n"  // $ for immediate, %% for register
        "mov %%ax, %%ds     \n"
        "mov %%ax, %%es     \n"
        "mov %%ax, %%fs     \n"
        "mov %%ax, %%gs     \n"

        "push $0x23         \n"
        "push %0            \n"
        "push $0x202        \n"
        "push $0x1B         \n"
        "push %1            \n"
        "iret               \n"
        :
        : "i"(USER_ESP), "i"(USER_EIP)
    );
}
__attribute__((section(".text.main")))
void main()
{
    // clear screen
    clear();

    kprintf("%CExecuting main() at address: %p\n", VGA_GREEN, VGA_BLACK, (void*)main);

    kprintf("Initializing Global Discriptor Table.\n");
    gdt_init();

    kprintf("Initializing TSS.\n");
    tss_init();
    
    // initializing the idt
    kprintf("Initializing Interrupt Discriptor Table.\n");
    idt_init();
    jump_to_usermode();

    __asm__("sti");
    kprintf("$ ");
    while(1)__asm__("hlt");
}

