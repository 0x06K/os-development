// Global Discriptor Table - this time implementing with my own two hands.

#include "gdt.h"
#include<stdint.h>

typedef struct{
    uint16_t LimitLow;
    uint16_t BaseLow;
    uint8_t BaseMiddle;
    uint8_t AccessBytes;
    uint8_t Limit_AND_Flags;
    uint8_t BaseHigh;
} __attribute__(packed) GLOBAL_DISCRIPTOR_TABLE;