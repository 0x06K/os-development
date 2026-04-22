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


__attribute__((section(".text.main")))
void main()
{
    // clear screen
    clear();
    kprintf("%CExecuting main() at address: %p\n", VGA_GREEN, VGA_BLACK, (void*)main);
    

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
    ls();
    
    file_info f = find_file("user.bin");
    if (f.start_cluster == 0) {
        kprintf("file not found\n");
    }
    kprintf("found at cluster=%d size=%d\n", f.start_cluster, f.size);
    
    uint8_t *load_addr = (uint8_t *)0x40000000;
    load_file(&f, load_addr);
    kprintf("loaded %d bytes at 0x40000000\n", f.size);
    // while(1);
    load_and_run(&f, (uint8_t*)USER_LOAD_ADDR, USER_STACK_ADDR);
    kprintf("$ ");
    while(1)__asm__("hlt");
}

