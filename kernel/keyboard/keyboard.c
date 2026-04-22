#include <stdint.h>
#include <vga/vga.h>
#include "keyboard.h"

static const char scancode_table[128] = {
    0,   0,  '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' '
};

#define BUF_SIZE 256
static volatile char     kbd_buf[BUF_SIZE];
static volatile int      kbd_head = 0;
static volatile int      kbd_tail = 0;

void keyboard_push(uint8_t scancode)
{
    if (scancode & 0x80) return;              // ignore key release
    if (scancode >= 128)  return;
    char c = scancode_table[scancode];
    if (!c) return;

    int next = (kbd_head + 1) % BUF_SIZE;
    if (next == kbd_tail) return;             // buffer full, drop
    kbd_buf[kbd_head] = c;
    kbd_head = next;
}

char kgetchar(void)
{
    while (kbd_head == kbd_tail){    __asm__ volatile ("sti; hlt" : : : "memory"); }             // block until key available
    char c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % BUF_SIZE;
    return c;
}

void kgets(char *buf, int max)
{
    int i = 0;
    while (1)
    {
        char c = kgetchar();
        if (c == '\n') { buf[i] = 0; return; }
        if (c == '\b') {
            if (i > 0) { i--; kprintf("\b"); }  // erase last char
            continue;
        }
        if (i < max - 1) {
            buf[i++] = c;
            kprintf("%c", c);                       // echo to screen
        }
    }
}

int kstrlen(char *s)
{
    int i = 0; while (s[i]) i++; return i;
}