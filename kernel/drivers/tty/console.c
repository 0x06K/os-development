// console.c
#include <drivers/tty/vga.h>
#include <drivers/tty/console.h>
#include <drivers/tty/serial.h>


void console_init(void) {
    serial_init();
    vga_initialize();
}

void console_putc(char c) {
    vga_putchar(c);
    serial_putc(c);
}

void console_write(const char *str) {
    while (*str) {
        console_putc(*str++);
    }
}

void console_clear(void) {
    vga_clear();
    serial_write("\n\n");
}

static void print_num(int num, int base) {
    char buf[32];
    int i = 0;
    int neg = 0;
    
    if (num == 0) {
        console_putc('0');
        return;
    }
    
    if (num < 0 && base == 10) {
        neg = 1;
        num = -num;
    }
    
    while (num) {
        int rem = num % base;
        buf[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num /= base;
    }
    
    if (neg) buf[i++] = '-';
    
    while (i > 0) {
        console_putc(buf[--i]);
    }
}