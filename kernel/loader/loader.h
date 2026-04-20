#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
void load_file(file_info *f, uint8_t *dest);
void load_and_run(file_info *f, uint8_t *dest, uint32_t user_stack);
#endif