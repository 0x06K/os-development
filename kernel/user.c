// user.c
// compile with: gcc -m32 -nostdlib -nostdinc -fno-builtin
//               -fno-stack-protector -T user.ld -o user.elf user.c
// then: objcopy -O binary user.elf user.bin

void user_program(void) {
    char *msg = "Hello Friend";

    asm volatile(
        "int $0x80"
        :
        : "a"(1), "b"(msg)
    );

    while(1);
}