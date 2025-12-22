#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

// Base I/O port for COM1 (you can adjust if using COM2, COM3, etc.)
#define COM1_PORT 0x3F8

// Initializes the serial port (baud rate, line control, etc.)
void serial_init(void);

// Sends one character through the serial port
void serial_putc(char c);

// Sends a null-terminated string through the serial port
void serial_write(const char *str);

// Returns 1 if a character is available to read, 0 otherwise
int serial_received(void);

// Reads one character from the serial port (blocking)
char serial_read(void);

#endif // SERIAL_H
