#include <stdint.h>
#include "ports.h"
#include "vga.h"
#include "timer_isr.h"
#include "scheduler.h"

volatile uint32_t system_ticks = 0;

void isr0_handler_c()
{
    system_ticks++;
    outb(0x20, 0x20);   // send EOI to master PIC
}

void isr_install(){
  idt_set_gate(32, (uint32_t)isr0, 0x08, 0x8E);
}
