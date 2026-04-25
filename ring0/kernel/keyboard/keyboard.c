#include <stdint.h>
#include <vga/vga.h>
#include "keyboard.h"

// ─────────────────────────────────────────────────────────────────────────────
// US LAYOUT — normal and shifted tables
// ─────────────────────────────────────────────────────────────────────────────

static const char normal[128] = {
/*00*/  0,    0,   '1', '2', '3', '4', '5', '6',
/*08*/ '7',  '8', '9', '0', '-', '=',  '\b', '\t',
/*10*/ 'q',  'w', 'e', 'r', 't', 'y', 'u',  'i',
/*18*/ 'o',  'p', '[', ']', '\n', 0,  'a',  's',
/*20*/ 'd',  'f', 'g', 'h', 'j', 'k', 'l',  ';',
/*28*/ '\'', '`',  0,  '\\','z', 'x', 'c',  'v',
/*30*/ 'b',  'n', 'm', ',', '.', '/',  0,   '*',
/*38*/  0,   ' ',  0,   0,   0,   0,   0,    0,
/*40*/  0,    0,   0,   0,   0,   0,   0,   '7',
/*48*/ '8',  '9', '-', '4', '5', '6', '+',  '1',
/*50*/ '2',  '3', '0', '.',  0,   0,   0,    0,
/*58*/  0,    0,   0,   0,   0,   0,   0,    0,
/*60*/  0,    0,   0,   0,   0,   0,   0,    0,
/*68*/  0,    0,   0,   0,   0,   0,   0,    0,
/*70*/  0,    0,   0,   0,   0,   0,   0,    0,
/*78*/  0,    0,   0,   0,   0,   0,   0,    0,
};

static const char shifted[128] = {
/*00*/  0,    0,   '!', '@', '#', '$', '%', '^',
/*08*/ '&',  '*', '(', ')', '_', '+',  '\b', '\t',
/*10*/ 'Q',  'W', 'E', 'R', 'T', 'Y', 'U',  'I',
/*18*/ 'O',  'P', '{', '}', '\n', 0,  'A',  'S',
/*20*/ 'D',  'F', 'G', 'H', 'J', 'K', 'L',  ':',
/*28*/ '"',  '~',  0,  '|', 'Z', 'X', 'C',  'V',
/*30*/ 'B',  'N', 'M', '<', '>', '?',  0,   '*',
/*38*/  0,   ' ',  0,   0,   0,   0,   0,    0,
/*40*/  0,    0,   0,   0,   0,   0,   0,   '7',
/*48*/ '8',  '9', '-', '4', '5', '6', '+',  '1',
/*50*/ '2',  '3', '0', '.',  0,   0,   0,    0,
/*58*/  0,    0,   0,   0,   0,   0,   0,    0,
/*60*/  0,    0,   0,   0,   0,   0,   0,    0,
/*68*/  0,    0,   0,   0,   0,   0,   0,    0,
/*70*/  0,    0,   0,   0,   0,   0,   0,    0,
/*78*/  0,    0,   0,   0,   0,   0,   0,    0,
};

// ─────────────────────────────────────────────────────────────────────────────
// SCANCODES FOR MODIFIER KEYS
// ─────────────────────────────────────────────────────────────────────────────

#define SC_LSHIFT       0x2A
#define SC_RSHIFT       0x36
#define SC_LSHIFT_REL   0xAA   // 0x2A | 0x80
#define SC_RSHIFT_REL   0xB6   // 0x36 | 0x80
#define SC_CAPSLOCK     0x3A

// ─────────────────────────────────────────────────────────────────────────────
// STATE
// ─────────────────────────────────────────────────────────────────────────────

static volatile int shift_held  = 0;
static volatile int caps_active = 0;

#define BUF_SIZE 256
static volatile char kbd_buf[BUF_SIZE];
static volatile int  kbd_head = 0;
static volatile int  kbd_tail = 0;

// ─────────────────────────────────────────────────────────────────────────────
// keyboard_push — called from IRQ1 handler
// ─────────────────────────────────────────────────────────────────────────────

void keyboard_push(uint8_t scancode) {

    // ── modifier tracking ───────────────────────────────────────────────────
    if (scancode == SC_LSHIFT || scancode == SC_RSHIFT) {
        shift_held = 1; return;
    }
    if (scancode == SC_LSHIFT_REL || scancode == SC_RSHIFT_REL) {
        shift_held = 0; return;
    }
    if (scancode == SC_CAPSLOCK) {
        caps_active = !caps_active; return;
    }

    // ── ignore key release and out-of-range ─────────────────────────────────
    if (scancode & 0x80) return;
    if (scancode >= 128)  return;

    // ── pick table ──────────────────────────────────────────────────────────
    char c;
    int is_letter = (normal[scancode] >= 'a' && normal[scancode] <= 'z');

    if (is_letter) {
        // caps lock only affects letters; shift inverts caps state for letters
        int upper = caps_active ^ shift_held;
        c = upper ? shifted[scancode] : normal[scancode];
    } else {
        // for non-letters only shift matters, caps lock has no effect
        c = shift_held ? shifted[scancode] : normal[scancode];
    }

    if (!c) return;

    // ── push into ring buffer ───────────────────────────────────────────────
    int next = (kbd_head + 1) % BUF_SIZE;
    if (next == kbd_tail) return;   // full, drop
    kbd_buf[kbd_head] = c;
    kbd_head = next;
}

// ─────────────────────────────────────────────────────────────────────────────
// kgetchar — block until a character is available
// ─────────────────────────────────────────────────────────────────────────────

char kgetchar(void) {
    while (kbd_head == kbd_tail)
        __asm__ volatile ("sti; hlt" ::: "memory");

    char c   = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % BUF_SIZE;
    return c;
}

// ─────────────────────────────────────────────────────────────────────────────
// kgets — read a line, echo to screen, handle backspace
// ─────────────────────────────────────────────────────────────────────────────

void kgets(char *buf, int max) {
    int i = 0;
    while (1) {
        char c = kgetchar();

        if (c == '\n') {
            kprintf("\n");
            buf[i] = '\0';
            return;
        }

        if (c == '\b') {
            if (i > 0) {
                i--;
                kprintf("\b \b");   // erase character on screen
            }
            continue;
        }

        if (i < max - 1) {
            buf[i++] = c;
            kprintf("%c", c);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// kstrlen
// ─────────────────────────────────────────────────────────────────────────────

int kstrlen(char *s) {
    int i = 0; while (s[i]) i++; return i;
}