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
    uint8_t Limit;
    uint16_t Base;
} __attribute__(packed) GDT_PTR;

