#ifndef PIC_H
#define PIC_H
#include "config.h"


void load_idt(uint32_t idt_ptr);

void idt_set_gate(uint8_t, uint32_t, uint16_t, uint8_t);
void idt_init();

#endif
