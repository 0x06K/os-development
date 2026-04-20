// user.c
// compile with: gcc -m32 -nostdlib -nostdinc -fno-builtin
//               -fno-stack-protector -T user.ld -o user.elf user.c
// then: objcopy -O binary user.elf user.bin

void user_program(void) {
    char *msg = "Hello Friend";

    __asm__ volatile (
        "int $0x80"
        :                        // no outputs
        : "a"(1),                // EAX = syscall number
          "b"(msg)               // EBX = string pointer
        : "memory"
    );

    while(1);
}