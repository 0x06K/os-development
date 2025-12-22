#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

#include <stdint.h>

// Basic colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_WHITE = 15,
};

// Initialize VGA text mode
void vga_initialize(void);

// Clear the screen
void vga_clear(void);

// Write one character
void vga_putchar(char c);

// Write a string (null-terminated)
void vga_writestring(const char *str);

//  scroll screen
void vga_scroll(void);

// Set text color
void vga_setcolor(uint8_t color);

#endif // VGA_H
 
