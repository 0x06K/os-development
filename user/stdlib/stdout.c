#include <stdarg.h>
#include <stdint.h>
#include "stdio.h"

#define SYS_KPRINTF 1

static uint8_t  _buf[1024];
static uint8_t *_cur;

static void emit(char c) {
    if (_cur < _buf + sizeof(_buf) - 1)
        *_cur++ = c;
}

static void buf_reverse(char *buf, int len) {
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
    if (!flag_left && pc == ' ') for (int i = 0; i < pad; i++) emit(' ');
    for (int i = 0; i < prefix_len; i++) emit(prefix[i]);
    if (!flag_left && pc == '0') for (int i = 0; i < pad; i++) emit('0');
    for (int i = 0; i < buf_len;    i++) emit(buf[i]);
    if  (flag_left)              for (int i = 0; i < pad; i++) emit(' ');
}

void printf(const char *fmt, ...) {
    _cur = _buf;

    va_list ap;
    va_start(ap, fmt);

    for (char *p = (char *)fmt; *p; p++) {
        if (*p != '%') { emit(*p); continue; }
        p++;

        int flag_left  = 0, flag_zero  = 0;
        int flag_plus  = 0, flag_space = 0, flag_hash = 0;
        while (*p == '-' || *p == '0' || *p == '+' || *p == ' ' || *p == '#') {
            if (*p == '-') flag_left  = 1;
            if (*p == '0') flag_zero  = 1;
            if (*p == '+') flag_plus  = 1;
            if (*p == ' ') flag_space = 1;
            if (*p == '#') flag_hash  = 1;
            p++;
        }

        int width = 0;
        while (*p >= '0' && *p <= '9') width = width * 10 + (*p++ - '0');

        int precision = -1;
        if (*p == '.') { p++; precision = 0; while (*p >= '0' && *p <= '9') precision = precision * 10 + (*p++ - '0'); }

        int is_long = 0;
        if      (*p == 'l') { p++; if (*p == 'l') p++; is_long = 1; }
        else if (*p == 'h') p++;

        char spec = *p;
        if (!spec) break;

        if (spec == 'd' || spec == 'i') {
            long val = is_long ? va_arg(ap, long) : (long)va_arg(ap, int);
            char prefix[4]; int prefix_len = 0;
            char buf[32];   int buf_len    = 0;

            if      (val < 0)    { prefix[prefix_len++] = '-'; val = -val; }
            else if (flag_plus)    prefix[prefix_len++] = '+';
            else if (flag_space)   prefix[prefix_len++] = ' ';

            unsigned long uval = (unsigned long)val;
            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = '0' + (uval % 10); uval /= 10; }
            buf_reverse(buf, buf_len);

            pad_print(prefix, prefix_len, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'u') {
            unsigned long uval = is_long ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
            char buf[32]; int buf_len = 0;

            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = '0' + (uval % 10); uval /= 10; }
            buf_reverse(buf, buf_len);

            pad_print("", 0, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'o') {
            unsigned long uval = is_long ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
            char prefix[4]; int prefix_len = 0;
            char buf[32];   int buf_len    = 0;

            if (flag_hash && uval) prefix[prefix_len++] = '0';

            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = '0' + (uval % 8); uval /= 8; }
            buf_reverse(buf, buf_len);

            pad_print(prefix, prefix_len, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'x' || spec == 'X') {
            const char *digits = (spec == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
            unsigned long uval = is_long ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
            char prefix[4]; int prefix_len = 0;
            char buf[32];   int buf_len    = 0;

            if (flag_hash && uval) { prefix[prefix_len++] = '0'; prefix[prefix_len++] = (spec == 'X') ? 'X' : 'x'; }

            if (uval == 0) buf[buf_len++] = '0';
            else while (uval) { buf[buf_len++] = digits[uval % 16]; uval /= 16; }
            buf_reverse(buf, buf_len);

            pad_print(prefix, prefix_len, buf, buf_len, width, flag_left, flag_zero);
        }
        else if (spec == 'p') {
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
        else if (spec == 's') {
            char *s = va_arg(ap, char *);
            if (!s) s = "(null)";
            int len = 0; while (s[len]) len++;
            if (precision >= 0 && precision < len) len = precision;

            int pad = width > len ? width - len : 0;
            if (!flag_left) for (int i = 0; i < pad; i++) emit(' ');
            for (int i = 0; i < len; i++) emit(s[i]);
            if  (flag_left) for (int i = 0; i < pad; i++) emit(' ');
        }
        else if (spec == 'c') {
            char c = (char)va_arg(ap, int);
            int pad = width > 1 ? width - 1 : 0;
            if (!flag_left) for (int i = 0; i < pad; i++) emit(' ');
            emit(c);
            if  (flag_left) for (int i = 0; i < pad; i++) emit(' ');
        }
        else if (spec == '%') emit('%');
        else if (spec == 'n') (void)va_arg(ap, int *);
        else if (spec == 'C') { (void)va_arg(ap, int); (void)va_arg(ap, int); }
        else if (spec == 'R') { /* no-op */ }
    }

    va_end(ap);
    *_cur = '\0';

    write(_buf);
}