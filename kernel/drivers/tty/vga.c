// vga.c
#include <drivers/tty/vga.h>

static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
static uint8_t vga_color = VGA_COLOR_CYAN;
static int vga_row = 0;
static int vga_col = 0;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void vga_initialize(void) {
    vga_color = VGA_COLOR_LIGHT_GREY;
    vga_row = 0;
    vga_col = 0;
    vga_clear();
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {

        for (int x = 0; x < VGA_WIDTH; x++) {

            vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else {
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_color);
        vga_col++;
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
    }
    if (vga_row >= VGA_HEIGHT) vga_scroll();
}

void vga_scroll(void) {
    // Scroll
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }
    vga_row = VGA_HEIGHT - 1;
}
void vga_writestring(const char *str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_setcolor(uint8_t color) {
    vga_color = color;
}
