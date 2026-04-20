#ifndef SYSCALLS_H
#define SYSCALLS_H


#include <stdint.h>

void write(uint8_t* str);
void read(uint8_t* buffer, uint32_t len);

#endif