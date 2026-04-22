// user.c
// compile with: gcc -m32 -nostdlib -nostdinc -fno-builtin
//               -fno-stack-protector -T user.ld -o user.elf user.c
// then: objcopy -O binary user.elf user.bin
#include <syscalls/syscalls.h>

__attribute__((section(".text.main")))
void user_program(void) {
    uint8_t *msg = "Hello Friend";
    char buffer[12];
    write(msg);
    read(msg, 5);
    while(1);
}
