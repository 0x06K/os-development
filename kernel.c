void clear_screen() {
    unsigned char *vga = (unsigned char *)0xB8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vga[i]     = ' ';
        vga[i + 1] = 0x02; // green on black
    }
}

void print(const char *str) {
    unsigned char *vga = (unsigned char *)0xB8000;
    int i = 0;
    while (str[i] != '\0') {
        vga[i * 2]     = str[i];
        vga[i * 2 + 1] = 0x02; // green on black
        i++;
    }
}

void kernel_main() {
    clear_screen();
    print("Hello, friend.");
}
