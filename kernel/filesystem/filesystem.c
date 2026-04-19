#include<vga/vga.h>
#include "filesystem.h"



// ─── port I/O (compiler intrinsics or inline asm) ────────────
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("in %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("out %0, %1" :: "a"(val), "Nd"(port));
}

static inline void insw(uint16_t port, void *buf, uint32_t count) {
    __asm__ volatile ("rep insw" : "+D"(buf), "+c"(count) : "d"(port) : "memory");
}
static inline void outsw(uint16_t port, void *buf, uint32_t count) {
    __asm__ volatile ("rep outsw" : "+S"(buf), "+c"(count) : "d"(port) : "memory");
}

// ─── data ────────────────────────────────────────────────────
static uint32_t lba    = 0;

volatile uint8_t status = 0;

// ─── Step 1-7: send the read command ─────────────────────────
void read_sector(uint32_t lba_addr, uint8_t* buffer) {
    status = 0; lba = lba_addr;

    // Step 1: wait for BSY to clear
    while (inb(0x1F7) & 0x80);

    // Step 2: drive select + LBA bits 24-27
    outb(0x1F6, 0xE0 | ((lba_addr >> 24) & 0x0F));

    // Step 3: sector count
    outb(0x1F2, 1);

    // Steps 4-6: LBA address bytes
    outb(0x1F3, (lba_addr)      & 0xFF);   // bits 0-7
    outb(0x1F4, (lba_addr >> 8) & 0xFF);   // bits 8-15
    outb(0x1F5, (lba_addr >> 16)& 0xFF);   // bits 16-23

    // Step 7: fire the READ command
    outb(0x1F7, 0x20);

    while(!status) __asm__("hlt");

    insw(0x1F0, buffer, 256);
}

// ─── Step 1-7: send the write command ────────────────────────
void write_sector(uint32_t lba_addr, uint8_t* data) {

    // Step 1: wait for BSY to clear
    while (inb(0x1F7) & 0x80);

    // Step 2: drive select + LBA bits 24-27
    outb(0x1F6, 0xE0 | ((lba_addr >> 24) & 0x0F));

    // Step 3: sector count
    outb(0x1F2, 1);

    // Steps 4-6: LBA address bytes
    outb(0x1F3, (lba_addr)       & 0xFF);  // bits 0-7
    outb(0x1F4, (lba_addr >> 8)  & 0xFF);  // bits 8-15
    outb(0x1F5, (lba_addr >> 16) & 0xFF);  // bits 16-23

    // Step 7: fire the WRITE command (0x30 instead of 0x20)
    outb(0x1F7, 0x30);

    // wait for DRQ — drive saying "ok send me the data"
    while (!(inb(0x1F7) & 0x08));

    // push 256 words to the data port
    outsw(0x1F0, data, 256);

    // walk away — drive flushes to disk, fires IRQ14 when done
}

struct fat32_fs fs;


void fs_init() {
    uint8_t buf[512];
    read_sector(0, buf);

    struct bpb *b = (struct bpb *)buf;

    // verify it's actually a FAT32 volume
    if (b->bytes_per_sector == 0) {
        kprintf("fs_init: invalid BPB\n");
        return;
    }

    // calculate and store everything you need
    fs.bytes_per_sector    = b->bytes_per_sector;
    fs.sectors_per_cluster = b->sectors_per_cluster;
    fs.fat_start           = b->reserved_sectors;
    fs.data_start          = fs.fat_start + (b->fat_count * b->sectors_per_fat_32);
    fs.root_cluster        = b->root_cluster;

    kprintf("fs initialized\n");
    kprintf("fat_start  = %d\n", fs.fat_start);
    kprintf("data_start = %d\n", fs.data_start);
    kprintf("root_cluster = %d\n", fs.root_cluster);
}

/*  
    Note for me;
    (dir_entry*)(dir_buffer): treat the address(dir_buffer) as a pointer to a structure named dir_entry.
    (dir_entry)(dir_buffer): strting from the address(dir_buffer) treat the memory as a sturcture named dir_entry.

*/
uint32_t read_fat_entry(uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;                        // 4 bytes per FAT32 entry
    uint32_t fat_sector = fs.fat_start + (fat_offset / 512);
    uint32_t entry_offset = fat_offset % 512;

    uint8_t buf[512];
    read_sector(fat_sector, buf);

    return (*(uint32_t *)(buf + entry_offset)) & 0x0FFFFFFF;
}

void ls() {
    uint32_t cluster = fs.root_cluster;

    while (cluster < 0x0FFFFFF8) {        // 0x0FFFFFF8 = end of chain
        // convert cluster to sector
        uint32_t sector = fs.data_start + (cluster - 2) * fs.sectors_per_cluster;

        // read all sectors in this cluster
        for (uint32_t s = 0; s < fs.sectors_per_cluster; s++) {
            uint8_t dir_buffer[512];
            read_sector(sector + s, dir_buffer);

            struct dir_entry *entries = (struct dir_entry *)dir_buffer;

            for (int i = 0; i < 16; i++) {
                if (entries[i].name[0] == 0x00) return;  // end of directory
                if (entries[i].name[0] == (uint8_t)0xE5) continue; // deleted

                uint32_t entry_cluster = ((uint32_t)entries[i].cluster_high << 16) | entries[i].cluster_low;

                if (entries[i].attributes == 0x10)
                    kprintf("DIR:  %.8s cluster=%d\n", entries[i].name, entry_cluster);
                else
                    kprintf("FILE: %.8s.%.3s size=%d cluster=%d\n", entries[i].name, entries[i].ext, entries[i].size, entry_cluster);
            }
        }

        // follow FAT chain to next cluster
        cluster = read_fat_entry(cluster);
    }
}
