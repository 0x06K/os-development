#include <gdt/gdt.h>
#include "tss.h"

static struct _TSS tss;
static uint8_t kernel_stack[8192];   // 8KB kernel stack



void tss_init(void)
{
    // zero everything out
    for (uint32_t i = 0; i < sizeof(TSS); i++)
        ((uint8_t*)&tss)[i] = 0;

    tss.ss0  = 0x10;                                    // kernel data segment
    tss.esp0 = (uint32_t)kernel_stack + sizeof(kernel_stack); // top of kernel stack

    tss.iomap_base = sizeof(TSS);   // no IO map

    // add TSS descriptor into GDT as entry 5
    // base = address of tss, limit = sizeof(TSS)-1
    // access = 0x89 (present, ring 0, TSS type)
    // flags  = 0x0  (byte granularity, 16-bit)
    gdt_set_entry(5, (uint32_t)&tss, sizeof(TSS) - 1, 0x89, 0x0);

    // load TSS — selector 0x28 = GDT entry 5 (5 * 8 = 40 = 0x28)
    asm volatile("ltr %0" : : "r"((uint16_t)0x28));
}