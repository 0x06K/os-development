// user.c
// compile with: gcc -m32 -nostdlib -nostdinc -fno-builtin
//               -fno-stack-protector -T user.ld -o user.elf user.c
// then: objcopy -O binary user.elf user.bin
#include <syscalls/syscalls.h>
#include <shell/shell.h>

__attribute__((section(".text.main")))
void user_program(void) {
    const uint8_t *msg = "Hello Friend!!\n";
    clear();
    write(msg);
    shell_main();
    while(1);
}
