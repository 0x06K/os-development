#include <stdint.h>
#include <handlers/handlers.h>
#include "idt.h"


extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t value);



static IDT_Entry idt[256] = {0};
static IDT_Ptr   idt_ptr;



static void idt_set_entry(uint8_t idx, void *handler)
{
    uint32_t addr        = (uint32_t)handler;
    idt[idx].offset_low  = addr & 0xFFFF;
    idt[idx].selector    = 0x08;
    idt[idx].zero        = 0;
    idt[idx].type_attr   = 0x8E;
    idt[idx].offset_high = (addr >> 16) & 0xFFFF;
}



void idt_init(void)
{
    idt_ptr.limit = (sizeof(IDT_Entry) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0x00); outb(0xA1, 0x00);

    idt_set_entry(32, (void*)timer_handler);
    idt_set_entry(33, (void*)keyboard_handler);
    
    asm volatile("lidt %0" : : "m"(idt_ptr));
}

