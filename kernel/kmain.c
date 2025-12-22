#include <drivers/tty/vga.h>
#include <drivers/tty/serial.h>
#include <arch_x86_64/gdt_idt/gdt.h>
#include <kernel/mm/physical/pmm.h>

#define MEM_SIZE (4 * 1024 * 1024 * 1024)

void kmain(void) {
    // 1. Initialize serial port first — so we can log immediately
    // my dear me this is something that you don't need to focus on.
    // For now just skip it. i am just initializing later we can study it.
    // serial_init();

    // Initializing VGA for on-screen messages
    vga_initialize();
    vga_writestring("[+] vga_init();\n");

    // Initializing the GDT.
    gdt_init();
    vga_writestring("[+] gdt_init();\n");

    // Initializing the physical memory frames.
    pmm_init();
    vga_writestring("[+] pmm_init();\n");
    
    // 4. Halt CPU
    for (;;)
        __asm__ volatile ("hlt");
}