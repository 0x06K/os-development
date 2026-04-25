#pragma once
// filesystem.h — FAT32 driver public interface
// Only exposes the 18 syscall-level functions + init + necessary structs.
// All helpers are static inside filesystem.c and not visible here.

#include <stdint.h>

#define FS_MAX_PATH 256

// ─────────────────────────────────────────────────────────────────────────────
// ON-DISK STRUCTURES
// ─────────────────────────────────────────────────────────────────────────────

struct bpb {
    uint8_t  jump[3];
    uint8_t  oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sectors_per_fat_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
} __attribute__((packed));

struct dir_entry {
    uint8_t  name[8];
    uint8_t  ext[3];
    uint8_t  attributes;
    uint8_t  reserved;
    uint8_t  create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t cluster_low;
    uint32_t size;
} __attribute__((packed));

// ─────────────────────────────────────────────────────────────────────────────
// IN-MEMORY FS STATE
// ─────────────────────────────────────────────────────────────────────────────

struct fat32_fs {
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint32_t fat_start;
    uint32_t data_start;
    uint32_t root_cluster;
    uint32_t fat_size;
    uint32_t total_clusters;
};

extern struct fat32_fs fs;

// ─────────────────────────────────────────────────────────────────────────────
// RESULT TYPES
// ─────────────────────────────────────────────────────────────────────────────

struct stat_info {
    uint32_t size;
    uint32_t start_cluster;
    uint8_t  attributes;
    int      is_dir;
};

// ─────────────────────────────────────────────────────────────────────────────
// INIT
// ─────────────────────────────────────────────────────────────────────────────

void fs_init(void);

// ─────────────────────────────────────────────────────────────────────────────
// 18 COMMAND FUNCTIONS — these are the only things the shell/syscall layer sees
// ─────────────────────────────────────────────────────────────────────────────

// cat <file>
void        fs_cat(const char *name);

// touch <file>
int         fs_create(const char *name);

// rm <file>
int         fs_unlink(const char *name);

// mv <src> <dst>
int         fs_rename(const char *old_name, const char *new_name);

// cp <src> <dst>
int         fs_cp(const char *src, const char *dst);

// ls [cluster]
void        ls_dir(uint32_t dir_cluster);

// mkdir <name>
int         fs_mkdir(const char *name);

// mkdir -p <path>
int         fs_mkdir_p(const char *path);

// rmdir <name>
int         fs_rmdir(const char *name);

// rmdir -r <name>
int         fs_rmdir_r(const char *name);

// cd <path>
int         fs_chdir(const char *path);

// pwd
const char *fs_getcwd(void);

// stat <path>
int         fs_stat(const char *path, struct stat_info *out);

// find <name>
void        fs_find(const char *name);

// df
void        fs_df(void);

// du <file>
void        fs_du(const char *name);

// sync
void        fs_sync(void);

// echo <text> [> file]  — if filename is NULL, prints to terminal
int         fs_echo(const char *text, const char *filename);

int fs_read_file(const char *name, uint8_t *buffer, uint32_t offset, uint32_t len);