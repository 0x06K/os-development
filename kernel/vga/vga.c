#include "vga.h"


uint8_t current_color = (VGA_BLACK << 4) | VGA_GREEN;  // default white on black

static int cursor_x = 0;
static int cursor_y = 0;


static void vga_putchar(uint8_t c)
{
    unsigned short *vga = (unsigned short *)VGA_ADDRESS;
    if (c == '\n')
    {
        cursor_x = 0; cursor_y++;
    } 
    else if (c == '\b')
    {
        if (cursor_x > 0)
        {
            cursor_x--;
        }
        else if (cursor_y > 0)
        {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
        }

        vga[cursor_y * VGA_WIDTH + cursor_x] = ' ';
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


// ── kprintf helpers ───────────────────────────────────────────────────────────

static void buf_reverse(char *buf, int len)
{
    for (int i = 0, j = len - 1; i < j; i++, j--)
    { char t = buf[i]; buf[i] = buf[j]; buf[j] = t; }
}

static void pad_print(const char *prefix, int prefix_len,
                      const char *buf,    int buf_len,
                      int width, int flag_left, int flag_zero)
{
    int total = buf_len + prefix_len;
    int pad   = width > total ? width - total : 0;
    char pc   = (flag_zero && !flag_left) ? '0' : ' ';
    if (!flag_left && pc == ' ') for (int i = 0; i < pad; i++) vga_putchar(' ');
    for (int i = 0; i < prefix_len; i++) vga_putchar(prefix[i]);
    if (!flag_left && pc == '0') for (int i = 0; i < pad; i++) vga_putchar('0');
    for (int i = 0; i < buf_len;    i++) vga_putchar(buf[i]);
    if  (flag_left)              for (int i = 0; i < pad; i++) vga_putchar(' ');
}

// ── kprintf ───────────────────────────────────────────────────────────────────

void kprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    for (char *p = fmt; *p; p++)
    {
        if (*p != '%') { vga_putchar(*p); continue; }
        p++;

        // ── flags ─────────────────────────────────────────────────────────
        int flag_left  = 0;
        int flag_zero  = 0;
        int flag_plus  = 0;
        int flag_space = 0;
        int flag_hash  = 0;
        while (*p == '-' || *p == '0' || *p == '+' || *p == ' ' || *p == '#')
        {
            if (*p == '-') flag_left  = 1;
            if (*p == '0') flag_zero  = 1;
            if (*p == '+') flag_plus  = 1;
            if (*p == ' ') flag_space = 1;
            if (*p == '#') flag_hash  = 1;
            p++;
        }

        // ── width ─────────────────────────────────────────────────────────
        int width = 0;
        while (*p >= '0' && *p <= '9') width = width * 10 + (*p++ - '0');

        // ── precision ─────────────────────────────────────────────────────
        int precision = -1;
        if (*p == '.') { p++; precision = 0; while (*p >= '0' && *p <= '9') precision = precision * 10 + (*p++ - '0'); }

        // ── length modifier ───────────────────────────────────────────────
        int is_long = 0;
        if      (*p == 'l') { p++; if (*p == 'l') p++; is_long = 1; }
        else if (*p == 'h') p++;

        char spec = *p;
        if (!spec) break;

        // ── specifiers ────────────────────────────────────────────────────

        if (spec == 'd' || spec == 'i')
        {
            long val = is_long ? va_arg(ap, long) : (long)va_arg(ap, int);
            char prefix[4]; int prefix_len = 0;
            char buf[32];   int buf_len    = 0;

            if      (val < 0)     { prefix[prefix_len++] = '-'; val = -val; }
            else if (flag_plus)     prefix[prefix_len++] = '+';
            else if (flag_space)    prefix[prefix_len++] = ' ';

            unsigned long uval = (unsigned long)val;
            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = '0' + (uval % 10); uval /= 10; }
            buf_reverse(buf, buf_len);

            pad_print(prefix, prefix_len, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'u')
        {
            unsigned long uval = is_long ? va_arg(ap, unsigned long)
                                         : (unsigned long)va_arg(ap, unsigned int);
            char buf[32]; int buf_len = 0;

            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = '0' + (uval % 10); uval /= 10; }
            buf_reverse(buf, buf_len);

            pad_print("", 0, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'o')
        {
            unsigned long uval = is_long ? va_arg(ap, unsigned long)
                                         : (unsigned long)va_arg(ap, unsigned int);
            char prefix[4]; int prefix_len = 0;
            char buf[32];   int buf_len    = 0;

            if (flag_hash && uval) prefix[prefix_len++] = '0';

            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = '0' + (uval % 8); uval /= 8; }
            buf_reverse(buf, buf_len);

            pad_print(prefix, prefix_len, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'x' || spec == 'X')
        {
            const char *digits = (spec == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
            unsigned long uval = is_long ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);

            char prefix[4]; int prefix_len = 0;
            char buf[32];   int buf_len    = 0;

            if (flag_hash && uval)
            { prefix[prefix_len++] = '0'; prefix[prefix_len++] = (spec == 'X') ? 'X' : 'x'; }

            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = digits[uval % 16]; uval /= 16; }
            buf_reverse(buf, buf_len);

            pad_print(prefix, prefix_len, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'p')
        {
            unsigned long uval = (unsigned long)va_arg(ap, void *);
            char prefix[4]; int prefix_len = 0;
            char buf[32];   int buf_len    = 0;

            prefix[prefix_len++] = '0';
            prefix[prefix_len++] = 'x';

            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = "0123456789abcdef"[uval % 16]; uval /= 16; }
            buf_reverse(buf, buf_len);

            pad_print(prefix, prefix_len, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 's')
        {
            char *s = va_arg(ap, char *);
            if (!s) s = "(null)";
            int len = 0; while (s[len]) len++;
            if (precision >= 0 && precision < len) len = precision;

            int pad = width > len ? width - len : 0;
            if (!flag_left) for (int i = 0; i < pad; i++) vga_putchar(' ');
            for (int i = 0; i < len; i++) vga_putchar(s[i]);
            if  (flag_left) for (int i = 0; i < pad; i++) vga_putchar(' ');
        }
        else if (spec == 'c')
        {
            char c = (char)va_arg(ap, int);
            int pad = width > 1 ? width - 1 : 0;
            if (!flag_left) for (int i = 0; i < pad; i++) vga_putchar(' ');
            vga_putchar(c);
            if  (flag_left) for (int i = 0; i < pad; i++) vga_putchar(' ');
        }
        else if (spec == '%') vga_putchar('%');
        else if (spec == 'n') (void)va_arg(ap, int *);
        else if (spec == 'C')
        {
            uint8_t fg = (uint8_t)va_arg(ap, int);
            uint8_t bg = (uint8_t)va_arg(ap, int);
            current_color = (bg << 4) | fg;
        }
        else if (spec == 'R') current_color = 0x0F;
    }

    va_end(ap);
}

void clear(){
    unsigned short *vga = (unsigned short *)VGA_ADDRESS;
    for (int i = 0; i < VGA_HEIGHT; i++)
        for (int j = 0; j < VGA_WIDTH; j++)
            vga[i * VGA_WIDTH + j] = ' ';
}