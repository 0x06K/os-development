#include <vga/vga.h>
#include <idt/idt.h>

extern volatile int scancode;

__attribute__((section(".text.main")))
void main()
{
    // clear screen
    clear();

    printf("%CExecuting main() at address: %p\n", VGA_GREEN, VGA_BLACK, (void*)main);

    // initializing the idt
    idt_init();

    __asm__("sti");

    while(1){
        __asm__("hlt");  // loop until keyboard fires
    }
}

