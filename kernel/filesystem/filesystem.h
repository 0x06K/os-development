#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

struct __attribute__((packed)) bpb {
    uint8_t  jump[3];           // offset 0
    char     oem[8];            // offset 3
    uint16_t bytes_per_sector;  // offset 11
    uint8_t  sectors_per_cluster; // offset 13
    uint16_t reserved_sectors;  // offset 14
    uint8_t  fat_count;         // offset 16
    uint16_t root_entry_count;  // offset 17
    uint16_t total_sectors_16;  // offset 19
    uint8_t  media_type;        // offset 21
    uint16_t sectors_per_fat_16;// offset 22
    uint16_t sectors_per_track; // offset 24
    uint16_t head_count;        // offset 26
    uint32_t hidden_sectors;    // offset 28
    uint32_t total_sectors_32;  // offset 32
    uint32_t sectors_per_fat_32;// offset 36
    uint16_t flags;             // offset 40
    uint16_t version;           // offset 42
    uint32_t root_cluster;      // offset 44
    uint16_t fs_info_sector;    // offset 48
    uint16_t backup_boot_sector;// offset 50
};

struct fat32_fs {
    uint32_t fat_start;
    uint32_t data_start;
    uint32_t root_cluster;
    uint32_t sectors_per_cluster;
    uint32_t bytes_per_sector;
};

struct __attribute__((packed)) dir_entry {
    char     name[8];               // offset 0
    char     ext[3];                // offset 8
    uint8_t  attributes;            // offset 11
    uint8_t  reserved;              // offset 12
    uint8_t  creation_time_ms;      // offset 13
    uint16_t creation_time;         // offset 14
    uint16_t creation_date;         // offset 16
    uint16_t access_date;           // offset 18
    uint16_t cluster_high;          // offset 20
    uint16_t mod_time;              // offset 22
    uint16_t mod_date;              // offset 24
    uint16_t cluster_low;           // offset 26
    uint32_t size;                  // offset 28
};                                  // total = 32 bytes
void read_sector(uint32_t lba_addr, uint8_t* buffer);
void write_sector(uint32_t lba_addr, uint8_t *data);
void fs_init();
void ls();

#endif