#include <vga/vga.h>
#include <keyboard/keyboard.h>
#include <idt/idt.h>
#include <gdt/gdt.h>
#include <tss/tss.h>
#include <frames/frames.h>
#include <filesystem/filesystem.h>

extern void user_program(void);

#define USER_EIP (uint32_t)user_program
#define USER_ESP 0xBFFFFFF0

__attribute__((section(".text.main")))
void main()
{
    // clear screen
    clear();

    kprintf("%CExecuting main() at address: %p\n", VGA_GREEN, VGA_BLACK, (void*)main);
    
    // initializing the idt
    kprintf("Initializing Interrupt Discriptor Table.\n");
    idt_init();
    // initializing gdt 
    kprintf("Initializing Global Discriptor Table.\n");
    gdt_init();
    
    // initializing TSS
    kprintf("Initializing TSS.\n");
    tss_init();
    
    // initializing frames allocator.
    kprintf("Initializing Physical Frames Manager.\n");
    frame_init();

    __asm__("sti");

    fs_init();
    ls();

    kprintf("$ ");
    while(1)__asm__("hlt");
}

