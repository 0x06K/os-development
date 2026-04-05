#include "vga.h"


static uint8_t current_color = 0x0F;  // default white on black

static int cursor_x = 0;
static int cursor_y = 0;


static void vga_putchar(char c)
{
    unsigned short *vga = (unsigned short *)VGA_ADDRESS;
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        vga[cursor_y * VGA_WIDTH + cursor_x] = (current_color << 8) | c; // use current_color
        cursor_x++;
    }
    if (cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
    if (cursor_y >= VGA_HEIGHT)
    {
        for (int i = 0; i < VGA_HEIGHT - 1; i++)
            for (int j = 0; j < VGA_WIDTH; j++)
                vga[i * VGA_WIDTH + j] = vga[(i + 1) * VGA_WIDTH + j];
        for (int j = 0; j < VGA_WIDTH; j++)
            vga[(VGA_HEIGHT - 1) * VGA_WIDTH + j] = (current_color << 8) | ' ';
        cursor_y = VGA_HEIGHT - 1;
    }
}

static void vga_putstr(char *s)
{
    if (!s) s = "(null)";
    while (*s) vga_putchar(*s++);
}

static void vga_putnbr_base(unsigned long n, int base, int upper)
{
    char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char buf[64];
    int  i = 0;

    if (n == 0) { vga_putchar('0'); return; }
    while (n > 0) { buf[i++] = digits[n % base]; n /= base; }
    while (i--) vga_putchar(buf[i]);
}

static void vga_putint(int n)
{
    if (n < 0) { vga_putchar('-'); vga_putnbr_base((unsigned long)-n, 10, 0); }
    else        vga_putnbr_base((unsigned long)n, 10, 0);
}

static void vga_putfloat(double f)
{
    if (f < 0) { vga_putchar('-'); f = -f; }

    long long int_part = (long long)f;
    double    frac     = f - int_part;

    vga_putnbr_base((unsigned long)int_part, 10, 0);
    vga_putchar('.');

    for (int d = 0; d < 6; d++)
    {
        frac *= 10;
        int digit = (int)frac;
        vga_putchar('0' + digit);
        frac -= digit;
    }
}

void printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    for (char *p = fmt; *p; p++)
    {
        if (*p != '%')      { vga_putchar(*p); continue; }
        p++;
        if      (*p == 'd' || *p == 'i') vga_putint(va_arg(ap, int));
        else if (*p == 'u')              vga_putnbr_base(va_arg(ap, unsigned int), 10, 0);
        else if (*p == 'o')              vga_putnbr_base(va_arg(ap, unsigned int),  8, 0);
        else if (*p == 'x')              vga_putnbr_base(va_arg(ap, unsigned int), 16, 0);
        else if (*p == 'X')              vga_putnbr_base(va_arg(ap, unsigned int), 16, 1);
        else if (*p == 'p')             { vga_putstr("0x"); vga_putnbr_base((unsigned long)va_arg(ap, void*), 16, 1); }
        else if (*p == 's')              vga_putstr(va_arg(ap, char*));
        else if (*p == 'c')              vga_putchar((char)va_arg(ap, int));
        else if (*p == 'f')              vga_putfloat(va_arg(ap, double));
        else if (*p == '%')              vga_putchar('%');
        else if (*p == 'n')              (void)va_arg(ap, int*);
        else if (*p == 'C')             // %C — set color, takes uint8_t fg, uint8_t bg
        {
            uint8_t fg = (uint8_t)va_arg(ap, int);
            uint8_t bg = (uint8_t)va_arg(ap, int);
            current_color = (bg << 4) | fg;  // color byte = [bg | fg]
        }
        else if (*p == 'R')              current_color = 0x0F;  // %R — reset to default
    }

    va_end(ap);
}   

void clear(){
    unsigned short *vga = (unsigned short *)VGA_ADDRESS;
    for (int i = 0; i < VGA_HEIGHT; i++)
        for (int j = 0; j < VGA_WIDTH; j++)
            vga[i * VGA_WIDTH + j] = ' ';
}