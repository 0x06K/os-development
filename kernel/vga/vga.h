#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stdarg.h>

// VGA Properties
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_COLOR   0x0F

// color codes for foreground (low nibble of color byte)
#define VGA_BLACK   0x0
#define VGA_BLUE    0x1
#define VGA_GREEN   0x2
#define VGA_CYAN    0x3
#define VGA_RED     0x4
#define VGA_MAGENTA 0x5
#define VGA_BROWN   0x6
#define VGA_WHITE   0x7
#define VGA_GRAY         0x8
#define VGA_LIGHT_BLUE   0x9
#define VGA_LIGHT_GREEN  0xA
#define VGA_LIGHT_CYAN   0xB
#define VGA_LIGHT_RED    0xC
#define VGA_PINK         0xD
#define VGA_YELLOW       0xE
#define VGA_BRIGHT_WHITE 0xF



void clear();
void kprintf(const char *fmt, ...);

#endif