#include "syscalls.h"

void write(uint8_t* str){
    __asm__ volatile (
        "int $0x80"
        :                        // no outputs
        : "a"(0),                // EAX = syscall number
          "b"(str)               // EBX = string pointer
        : "memory"
    );
}

void read(uint8_t* buffer, uint32_t len) {
    __asm__ volatile (
        "int $0x80"
        :                        // no outputs
        : "a"(1),    // EAX = syscall number
          "b"(buffer),           // EBX = string pointer
          "c"(len)
        : "memory"
    );
}