// syscalls.h — user-side syscall stubs

#include <stdint.h>
#include "syscalls.h"

void write(const uint8_t* str){
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

// 2. cat <file>
void cat(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(2), "b"(name)
        : "memory");
}

// 3. touch <file>
void touch(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(3), "b"(name)
        : "memory");
}

// 4. rm <file>
void rm(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(4), "b"(name)
        : "memory");
}

// 5. mv <src> <dst>
void mv(uint8_t *src, uint8_t *dst) {
    __asm__ volatile ("int $0x80"
        :: "a"(5), "b"(src), "c"(dst)
        : "memory");
}

// 6. cp <src> <dst>
void cp(uint8_t *src, uint8_t *dst) {
    __asm__ volatile ("int $0x80"
        :: "a"(6), "b"(src), "c"(dst)
        : "memory");
}

// 7. ls
void ls(uint32_t dir_cluster) {
    __asm__ volatile ("int $0x80"
        :: "a"(7), "b"(dir_cluster)
        : "memory");
}

// 8. mkdir <n>
void mkdir(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(8), "b"(name)
        : "memory");
}

// 9. mkdir -p <path>
void mkdir_p(uint8_t *path) {
    __asm__ volatile ("int $0x80"
        :: "a"(9), "b"(path)
        : "memory");
}

// 10. rmdir <n>
void rmdir(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(10), "b"(name)
        : "memory");
}

// 11. rmdir -r <n>
void rmdir_r(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(11), "b"(name)
        : "memory");
}

// 12. cd <path>
void cd(uint8_t *path) {
    __asm__ volatile ("int $0x80"
        :: "a"(12), "b"(path)
        : "memory");
}

// 13. pwd — returns pointer to cwd string
uint8_t *pwd(void) {
    uint8_t *result;
    __asm__ volatile ("int $0x80"
        : "=a"(result)
        : "a"(13)
        : "memory");
    return result;
}

// 14. stat <path> <struct pointer>
void stat(uint8_t *path, struct stat_info *info) {
    __asm__ volatile ("int $0x80"
        :: "a"(14), "b"(path), "c"(info)
        : "memory");
}

// 15. find <name>
void find(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(15), "b"(name)
        : "memory");
}

// 16. df
void df(void) {
    __asm__ volatile ("int $0x80"
        :: "a"(16)
        : "memory");
}

// 17. du <file>
void du(uint8_t *name) {
    __asm__ volatile ("int $0x80"
        :: "a"(17), "b"(name)
        : "memory");
}

// 18. sync
void sync(void) {
    __asm__ volatile ("int $0x80"
        :: "a"(18)
        : "memory");
}

// 19. echo <text> [> file]  — pass NULL as filename to print to terminal
void echo(uint8_t *text, uint8_t *filename) {
    __asm__ volatile ("int $0x80"
        :: "a"(19), "b"(text), "c"(filename)
        : "memory");
}

void clear(){
    __asm__ volatile ("int $0x80" :: "a"(20) : "memory");
}