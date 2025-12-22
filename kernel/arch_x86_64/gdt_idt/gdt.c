// GDT - this time implementing with my own two hands.

#include "gdt.h"
#include<stdint.h>

// GDT ENTRY
typedef struct {
    uint16_t LimitLow;        // Segment limit (bits 0–15)
    uint16_t BaseLow;         // Base address (bits 0–15)
    uint8_t  BaseMiddle;      // Base address (bits 16–23)
    uint8_t  Access;          // Access flags (type, privilege, present)
    uint8_t  LimitHigh_Flags; // High 4 bits of limit + flags (granularity, size)
    uint8_t  BaseHigh;        // Base address (bits 24–31)
} __attribute__((packed)) GLOBAL_DISCRIPTOR_ENTRY;

// GDT Pointer
typedef struct {
    uint16_t Limit;
    uint32_t Base;
} __attribute__((packed)) GDT_PTR;

static GLOBAL_DISCRIPTOR_ENTRY gdt[GDT_ENTRIES];
static GDT_PTR gdt_ptr;

static void gdt_set_entry(int idx, uint32_t base, uint32_t limit,uint8_t access, uint8_t gran) {
    gdt[idx].base_low        = base & 0xFFFF;
    gdt[idx].base_mid        = (base >> 16) & 0xFF;
    gdt[idx].base_high       = (base >> 24) & 0xFF;

    gdt[idx].limit_low       = limit & 0xFFFF;
    gdt[idx].limit_high_flags = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    gdt[idx].access          = access;
}

void gdt_init(void) {
    gdt_ptr.limit = sizeof(gdt_entry_t) * GDT_ENTRIES - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    // Null descriptor
    gdt_set_entry(0, 0, 0, 0, 0);

    // Ring 0 (kernel) code/data
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF); // Code, DPL 0
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF); // Data, DPL 0

    // Ring 3 (user) code/data
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xCF); // Code, DPL 3
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xCF); // Data, DPL 3

    gdt_flush((uint32_t)&gdt_ptr);
}