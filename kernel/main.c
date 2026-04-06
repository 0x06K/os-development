#include <vga/vga.h>

__attribute__((section(".text.main")))
void main()
{
    // clear the screen
    clear();
    printf("%CExecuting main() at address: %p\n", VGA_GREEN, VGA_BLACK, (void*)main);

    __asm__("hlt");
}