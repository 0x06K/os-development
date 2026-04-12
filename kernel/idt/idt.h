
#ifndef IDT_H
#define IDT_H

#include<stdint.h>

typedef struct __attribute__((packed))
{
    uint16_t offset_low;   // lower 16 bits of handler address  (where to jump - part 1)
    uint16_t selector;     // 0x08 = kernel code segment        (which segment to switch to)
    uint8_t  zero;         // always 0                          (reserved by Intel)
    uint8_t  type_attr;    // 0x8E = present, ring0, int gate   (who can call it, how to behave)
    uint16_t offset_high;  // upper 16 bits of handler address  (where to jump - part 2)
} IDT_Entry;


typedef struct __attribute__((packed))
{
    uint16_t limit;   // size of the IDT in bytes - 1
    uint32_t base;    // address of the IDT array
} IDT_Ptr;

void idt_set_entry(uint8_t idx, void *handler, uint8_t type_attr);
void idt_init(void);

#endif