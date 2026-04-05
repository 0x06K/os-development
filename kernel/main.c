#include <vga/vga.h>


__attribute__((section(".text.main")))
void main()
{
    clear();
    // %C takes fg color, bg color
    printf("%CHello in red!%R\n",        VGA_LIGHT_RED,   VGA_BLACK);
    printf("%CWarning: %s%R\n",          VGA_YELLOW,      VGA_BLACK, "check this");
    printf("%CBlue on white bg%R\n",     VGA_BLUE,        VGA_WHITE);
    printf("%CGreen: %d%R\n",            VGA_LIGHT_GREEN, VGA_BLACK, 42);

    __asm__("cli");
    __asm__("hlt");
}