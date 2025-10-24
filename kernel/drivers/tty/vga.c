#include "vga.h"

static uint16_t* const VGA_BUFFER = (uint16_t*) VGA_MEMORY;
static uint8_t vga_color = (VGA_COLOR_BLACK << 4) | VGA_COLOR_LIGHT_GREY;
static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

// Combine color + character into one VGA cell
static inline uint16_t vga_entry(char c, uint8_t color) {
    return ((uint16_t)color << 8) | (uint16_t)c;
}

void vga_setcolor(uint8_t color) {
    vga_color = color;
}

void vga_clear(void) {
    for (uint16_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', vga_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_initialize(void) {
    vga_setcolor((VGA_COLOR_BLACK << 4) | VGA_COLOR_LIGHT_GREY);
    vga_clear();
}

static void vga_newline(void) {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= VGA_HEIGHT) {
        // Scroll up one line
        for (uint16_t y = 1; y < VGA_HEIGHT; y++) {
            for (uint16_t x = 0; x < VGA_WIDTH; x++) {
                VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];
            }
        }
        // Clear last line
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_newline();
        return;
    }

    VGA_BUFFER[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, vga_color);
    cursor_x++;

    if (cursor_x >= VGA_WIDTH) {
        vga_newline();
    }
}

void vga_writestring(const char *str) {
    while (*str) {
        vga_putchar(*str++);
    }
}
