#include "syscalls.h"

__attribute__((naked))
void write(uint8_t* str){
    __asm__ volatile (
        "int $0x80"
        :                        // no outputs
        : "a"(1),                // EAX = syscall number
          "b"(str)               // EBX = string pointer
        : "memory"
    );
}

__attribute__((naked))
void read(uint8_t* buffer, uint32_t len) {
    __asm__ volatile (
        "int $0x80"
        :                        // no outputs
        : "a"(2),    // EAX = syscall number
          "b"(buffer),           // EBX = string pointer
          "c"(len)
        : "memory"
    );
}