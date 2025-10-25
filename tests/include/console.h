#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

// Initializes the console system (calls serial_init, vga_init, etc.)
void console_init(void);

// Writes one character to the console (both VGA + serial)
void console_putc(char c);

// Writes a string (null-terminated)
void console_write(const char *str);

// Clears the screen (VGA usually, serial may just send \n)
void console_clear(void);


#endif // CONSOLE_H
