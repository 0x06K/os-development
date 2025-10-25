#include "vga.h"
#include "serial.h"

void kmain(void) {
    // 1. Initialize serial port first — so we can log immediately
    // my dear me this is something that you don't need to focus on.
    // For now just skip it. i am just initializing later we can study it.
    serial_init();

    // 2. Initialize VGA for on-screen messages
    vga_initialize();
    vga_setcolor(VGA_COLOR_GREEN);
    vga_writestring("[+] VGA initialized.");

    // 4. Halt CPU
    for (;;)
        __asm__ volatile ("hlt");
}