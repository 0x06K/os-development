#include "myshell.h"


void shell_init(void) {
    vga_writestring("\n=== MyShell Initialized ===\n");
    vga_writestring("Type 'help' for available commands\n\n");
    vga_writestring("myshell> ");
    while(1);
}
