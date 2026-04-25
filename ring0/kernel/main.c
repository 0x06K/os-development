#include <vga/vga.h>
#include <keyboard/keyboard.h>
#include <idt/idt.h>
#include <gdt/gdt.h>
#include <tss/tss.h>
#include <frames/frames.h>
#include <filesystem/filesystem.h>
#include <loader/loader.h>


#define USER_LOAD_ADDR  0x40000000   // where the program is loaded in memory
#define USER_STACK_ADDR 0x50000000   // top of user stack


__attribute__((section(".text.main"))) void kmain()
{
    // clear screen
    clear();
    kprintf("%CExecuting main() at address: %p\n", VGA_GREEN, VGA_BLACK, (void*)kmain);
    

    // initializing gdt 
    kprintf("Initializing Global Discriptor Table.\n");
    gdt_init();
    
    // initializing TSS
    kprintf("Initializing TSS.\n");
    tss_init();
    
    // initializing frames allocator.
    kprintf("Initializing Physical Frames Manager.\n");
    frame_init();

    // initializing the idt
    kprintf("Initializing Interrupt Discriptor Table.\n");
    idt_init();
    __asm__("sti");

    
    
    fs_init();

    
    struct stat_info info;
    fs_stat("user.bin", &info);
    int bytes = fs_read_file("user.bin", (uint8_t*)USER_LOAD_ADDR, 0, info.size);
        
    jump_usermode(USER_LOAD_ADDR, USER_STACK_ADDR);
    while(1)__asm__("hlt");
}

