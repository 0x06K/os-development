#include<filesystem/filesystem.h>
#include "loader.h"


extern struct fat32_fs fs;
void load_file(file_info *f, uint8_t *dest){
    uint32_t cluster = f->start_cluster;
    uint32_t bytes_remaining = f->size;

    while (cluster < 0x0FFFFFF8) {
        uint32_t sector = fs.data_start + (cluster - 2) * fs.sectors_per_cluster;
        for (uint32_t s = 0; s < fs.sectors_per_cluster; s++) {
            if (bytes_remaining == 0) return;
            uint32_t to_read = bytes_remaining < 512 ? bytes_remaining : 512;
            uint8_t buf[512];
            read_sector(sector + s, buf);

            for (uint32_t i = 0; i < to_read; i++)
                dest[i] = buf[i];   // no offset needed

            dest += to_read;        // advance dest only once
            bytes_remaining -= to_read;
        }
        cluster = read_fat_entry(cluster);
    }
}

void jump_usermode(uint32_t entry, uint32_t user_stack) {
    __asm__ volatile (
        "cli                    \n"  // disable interrupts

        // set segment registers to user data segment (ring 3)
        // 0x23 = GDT user data selector | RPL 3
        "mov $0x23, %%ax        \n"
        "mov %%ax, %%ds         \n"
        "mov %%ax, %%es         \n"
        "mov %%ax, %%fs         \n"
        "mov %%ax, %%gs         \n"

        // build fake iret frame on stack
        // iret pops: EIP, CS, EFLAGS, ESP, SS
        "push $0x23             \n"  // SS  — user stack segment
        "push %1                \n"  // ESP — user stack pointer
        "pushf                  \n"  // EFLAGS
        "pop %%eax              \n"
        "or $0x200, %%eax       \n"  // set IF flag (enable interrupts in ring 3)
        "push %%eax             \n"
        "push $0x1B             \n"  // CS  — user code segment (0x1B = ring 3)
        "push %0                \n"  // EIP — entry point of loaded program

        "iret                   \n"  // jump to ring 3
        :
        : "r"(entry), "r"(user_stack)
        : "eax", "memory"
    );
}

void load_and_run(file_info *f, uint8_t *dest, uint32_t user_stack) {
    jump_usermode((uint32_t)dest, user_stack);
}