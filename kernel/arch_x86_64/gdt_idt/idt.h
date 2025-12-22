#ifndef IDT_H
#define IDT_H

#include <stdint.h>   // for uint16_t, uint32_t, uint8_t

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS   0x1B  // index=3, CPL=3 (lowest 2 bits set)
#define USER_DS   0x23  // index=4, CPL=3 (CPL=current previlige level)

// ==== IDT Entry Structure ====
// Each entry describes a single interrupt/trap gate.
// Packed to match CPU layout exactly.
typedef struct __attribute__((packed)) {
    uint16_t base_low;   // Lower 16 bits of handler function address
    uint16_t selector;   // Kernel code segment selector (e.g. KERNEL_CS)
    uint8_t  zero;       // Must always be 0
    uint8_t  type_attr;  // Type and attributes (present, DPL, gate type)
    uint16_t base_high;  // Upper 16 bits of handler function address
} idt_entry_t;

// ==== IDT Pointer ====
// Address and size for lidt instruction.
typedef struct __attribute__((packed)) {
    uint16_t limit;      // Size of the table - 1
    uint32_t base;       // Linear address of the first idt_entry_t
} idt_ptr_t;



// Initialize the IDT and load it with lidt
void idt_init(void);

// Register a single interrupt handler in the table
// 'num' = interrupt vector (0–255)
// 'base' = address of handler function
// 'sel'  = segment selector (usually KERNEL_CS)
// 'flags' = type/attribute byte (e.g. 0x8E = present + ring0 + 32-bit interrupt gate)
void idt_set_entry(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// Assembly routine that loads the IDT pointer (implemented in idt.asm)
extern void idt_flush(uint32_t idt_ptr_address);

#endif // IDT_H
