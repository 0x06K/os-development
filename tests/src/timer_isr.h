#ifndef ISR_H
#define ISR_H
#include <stdint.h>
#include "idt.h"

extern void isr0();
void isr_install();

#endif
