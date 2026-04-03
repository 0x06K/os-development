#include <stdint.h>

volatile uint16_t *vga = (volatile uint16_t *)0xB8000;

void main(void)
{
    char *msg = "Hello friend!!";
    uint8_t attr = 0x0F;

    for (int i = 0; msg[i]; i++)
        vga[i] = (uint16_t)(attr << 8) | msg[i];

    for (;;)
        __asm__ volatile ("hlt");
}