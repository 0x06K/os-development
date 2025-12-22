#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Null, Kernel Code, Kernel Data, User Code, User Data
#define GDT_ENTRIES 5



extern void gdt_flush(uint32_t);  // defined in ASM
void gdt_set_tss_entry(int, uint32_t, uint32_t, uint8_t, uint8_t);
void gdt_init(void);

#endif
