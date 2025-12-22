#include <stdint.h>
#include "idt.h"
#include "timer_isr.h"
#include "pic.h"

// IDT entry structure
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr   idtp;

// External function to load IDT (assembly)
extern void load_idt(uint32_t);

// Set one gate in the IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].sel      = sel;
    idt[num].always0  = 0;
    idt[num].flags    = flags;
    idt[num].base_high= (base >> 16) & 0xFFFF;
}
extern void pic_init();
// Initialize IDT
void idt_init() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (uint32_t)&idt;

    // Clear all entries
    for(int i = 0; i < 256; i++)
        idt_set_gate(i, 0, 0, 0);

    pic_init();
    // Install ISRs 0-31
    isr_install();

    // Load IDT
    load_idt((uint32_t)&idtp);
}
