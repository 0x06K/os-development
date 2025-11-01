#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Null, Kernel Code, Kernel Data, User Code, User Data
#define GDT_ENTRIES 5


static void gdt_set_entry(int idx, uint32_t base, uint32_t limit,uint8_t access, uint8_t gran);
void gdt_init(void);

#endif

