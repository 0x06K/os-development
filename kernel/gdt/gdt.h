#ifndef GDT_H
#define GDT_H

#include<stdint.h>


typedef struct __attribute__((packed))
{
    uint16_t limit_low;    // lower 16 bits of segment size
    uint16_t base_low;     // lower 16 bits of segment start address
    uint8_t  base_mid;     // middle 8 bits of segment start address
    uint8_t  access;       // who can use this segment and how
    uint8_t  flags_limit;  // upper 4 bits = flags, lower 4 bits = upper limit
    uint8_t  base_high;    // upper 8 bits of segment start address
} GDT_Entry;

typedef struct __attribute__((packed))
{
    uint16_t limit;   // size of GDT in bytes - 1
    uint32_t base;    // address of the GDT array
} GDT_Ptr;


void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_init();



#endif