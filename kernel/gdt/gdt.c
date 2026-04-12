#include <vga/vga.h>
#include "gdt.h"

#define GDT_ENTRIES 6

static GDT_Entry gdt[GDT_ENTRIES];
static GDT_Ptr   gdt_ptr;


void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    gdt[i].limit_low   =  limit & 0xFFFF;
    gdt[i].base_low    =  base  & 0xFFFF;
    gdt[i].base_mid    = (base  >> 16) & 0xFF;
    gdt[i].access      =  access;
    gdt[i].flags_limit = ((limit >> 16) & 0x0F) | (flags << 4);
    gdt[i].base_high   = (base  >> 24) & 0xFF;
}

void gdt_init(void)
{
    gdt_set_entry(0, 0,          0,          0x00, 0x0); // null
    gdt_set_entry(1, 0, 0x003FFFFF, 0x9A, 0xC); // kernel code  base=64KB  limit=4MB
    gdt_set_entry(2, 0, 0x003FFFFF, 0x92, 0xC); // kernel data  base=64KB  limit=4MB
    gdt_set_entry(3, 0x00400000, 0xFFFFFFFF, 0xFA, 0xC); // user code    base=4MB   limit=2GB
    gdt_set_entry(4, 0x00400000, 0xFFFFFFFF, 0xF2, 0xC); // user data    base=4MB   limit=2GB

    gdt_ptr.limit = (sizeof(GDT_Entry) * GDT_ENTRIES) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    kprintf("GDT base: %p\n", gdt_ptr.base);
    kprintf("GDT ptr: %p\n", &gdt_ptr);

    asm volatile("lgdt %0" : : "m"(gdt_ptr));
    kprintf("GDT loaded successfully.\n");
}