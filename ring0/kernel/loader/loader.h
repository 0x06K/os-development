#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
void jump_usermode(uint32_t entry, uint32_t user_stack);

#endif