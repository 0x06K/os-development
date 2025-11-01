#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Null, Kernel Code, Kernel Data, User Code, User Data
#define GDT_ENTRIES 5
static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t   gdt_ptr;

static void gdt_set_entry(int idx, uint32_t base, uint32_t limit,uint8_t access, uint8_t gran);
extern void gdt_flush(uint32_t);  // defined in ASM

void gdt_init(void);

#endif

