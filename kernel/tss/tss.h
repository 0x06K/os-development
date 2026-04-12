#ifndef TSS_H
#define TSS_H

#include <stdint.h>

typedef struct __attribute__((packed)) _TSS
{
    uint32_t prev_tss;   // unused, always 0
    uint32_t esp0;       // kernel stack pointer (ring 0 stack)
    uint32_t ss0;        // kernel stack segment (0x10 = kernel data)
    uint32_t esp1;       // unused
    uint32_t ss1;        // unused
    uint32_t esp2;       // unused
    uint32_t ss2;        // unused
    uint32_t cr3;        // unused (paging)
    uint32_t eip;        // unused
    uint32_t eflags;     // unused
    uint32_t eax;        // unused
    uint32_t ecx;        // unused
    uint32_t edx;        // unused
    uint32_t ebx;        // unused
    uint32_t esp;        // unused
    uint32_t ebp;        // unused
    uint32_t esi;        // unused
    uint32_t edi;        // unused
    uint32_t es;         // unused
    uint32_t cs;         // unused
    uint32_t ss;         // unused
    uint32_t ds;         // unused
    uint32_t fs;         // unused
    uint32_t gs;         // unused
    uint32_t ldt;        // unused
    uint16_t trap;       // unused
    uint16_t iomap_base; // unused
}TSS;

void tss_init(void);

#endif