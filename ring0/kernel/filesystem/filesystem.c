// filesystem.c — FAT32 driver
// Public interface: 18 command-backing functions declared in filesystem.h
// Everything else is static — invisible outside this file.

#include <vga/vga.h>
#include "filesystem.h"
#include <stdint.h>
#include <stddef.h>

// ─────────────────────────────────────────────────────────────────────────────
// GLOBALS
// ─────────────────────────────────────────────────────────────────────────────

volatile uint8_t irq_fired = 0;
struct fat32_fs  fs;

static uint32_t cwd_cluster = 2;
static char     cwd_path[FS_MAX_PATH] = "/";


// ─────────────────────────────────────────────────────────────────────────────
// PORT I/O  (static — hardware detail)
// ─────────────────────────────────────────────────────────────────────────────

static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile ("in %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static inline void outb(uint16_t port, uint8_t v) {
    __asm__ volatile ("out %0, %1" :: "a"(v), "Nd"(port));
}

static inline void insw(uint16_t port, void *buf, uint32_t count) {
    __asm__ volatile ("rep insw" : "+D"(buf), "+c"(count) : "d"(port) : "memory");
}

static inline void outsw(uint16_t port, void *buf, uint32_t count) {
    __asm__ volatile ("rep outsw" : "+S"(buf), "+c"(count) : "d"(port) : "memory");
}

// ─────────────────────────────────────────────────────────────────────────────
// SECTOR I/O  (static)
// ─────────────────────────────────────────────────────────────────────────────

static void read_sector(uint32_t lba, uint8_t *buf) {
    irq_fired = 0;
    while (inb(0x1F7) & 0x80);
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (lba)       & 0xFF);
    outb(0x1F4, (lba >>  8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x20);
    while (!irq_fired) __asm__("sti; hlt");
    insw(0x1F0, buf, 256);
}

static void write_sector(uint32_t lba, uint8_t *data) {
    while (inb(0x1F7) & 0x80);
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (lba)       & 0xFF);
    outb(0x1F4, (lba >>  8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F7, 0x30);
    while (!(inb(0x1F7) & 0x08));
    outsw(0x1F0, data, 256);
    outb(0x1F7, 0xE7);
    while (inb(0x1F7) & 0x80);
}

int fs_write_file(const char *name, const uint8_t *data, uint32_t offset, uint32_t len);

// ─────────────────────────────────────────────────────────────────────────────
// STRING / MEMORY HELPERS  (static)
// ─────────────────────────────────────────────────────────────────────────────

static int str_len(const char *s) {
    int i = 0; while (s[i]) i++; return i;
}

static void str_cpy(char *dst, const char *src) {
    while ((*dst++ = *src++));
}

static void str_cat(char *dst, const char *src) {
    str_cpy(dst + str_len(dst), src);
}

static int str_cmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static void mem_set(void *dst, uint8_t val, uint32_t len) {
    uint8_t *p = (uint8_t *)dst;
    while (len--) *p++ = val;
}

static void mem_cpy(void *dst, const void *src, uint32_t len) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    while (len--) *d++ = *s++;
}

// ─────────────────────────────────────────────────────────────────────────────
// 8.3 NAME HELPERS  (static)
// ─────────────────────────────────────────────────────────────────────────────

static void to_83(const char *input, char out[11]) {
    mem_set(out, ' ', 11);
    int i = 0, j = 0;
    while (input[j] && input[j] != '.' && i < 8) {
        char c = input[j++];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[i++] = c;
    }
    while (input[j] && input[j] != '.') j++;
    if (input[j] == '.') j++;
    for (int k = 0; k < 3 && input[j]; k++) {
        char c = input[j++];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[8 + k] = c;
    }
}

static int match_83(const char *input, const char *fat_name) {
    char conv[11];
    to_83(input, conv);
    for (int k = 0; k < 11; k++)
        if (conv[k] != fat_name[k]) return 0;
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// CLUSTER HELPERS  (static)
// ─────────────────────────────────────────────────────────────────────────────

static inline uint32_t cluster_to_sector(uint32_t cluster) {
    return fs.data_start + (cluster - 2) * fs.sectors_per_cluster;
}

static uint32_t read_fat_entry(uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fs.fat_start + (fat_offset / 512);
    uint32_t entry_off  = fat_offset % 512;
    uint8_t buf[512];
    read_sector(fat_sector, buf);
    return (*(uint32_t *)(buf + entry_off)) & 0x0FFFFFFF;
}

static void write_fat_entry(uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fs.fat_start + (fat_offset / 512);
    uint32_t entry_off  = fat_offset % 512;
    uint8_t buf[512];
    read_sector(fat_sector, buf);
    uint32_t *entry = (uint32_t *)(buf + entry_off);
    *entry = (*entry & 0xF0000000) | (value & 0x0FFFFFFF);
    write_sector(fat_sector, buf);
    write_sector(fat_sector + fs.fat_size, buf);  // mirror FAT2
}

static uint32_t alloc_cluster(void) {
    for (uint32_t c = 2; c < fs.total_clusters + 2; c++) {
        if (read_fat_entry(c) == 0x00000000) {
            write_fat_entry(c, 0x0FFFFFFF);
            return c;
        }
    }
    return 0;
}

static void free_cluster_chain(uint32_t start) {
    while (start < 0x0FFFFFF8 && start >= 2) {
        uint32_t next = read_fat_entry(start);
        write_fat_entry(start, 0x00000000);
        start = next;
    }
}

static void zero_cluster(uint32_t cluster) {
    uint8_t zeros[512];
    mem_set(zeros, 0, 512);
    uint32_t sector = cluster_to_sector(cluster);
    for (uint32_t s = 0; s < fs.sectors_per_cluster; s++)
        write_sector(sector + s, zeros);
}

static uint32_t extend_chain(uint32_t last) {
    uint32_t nc = alloc_cluster();
    if (!nc) return 0;
    write_fat_entry(last, nc);
    write_fat_entry(nc, 0x0FFFFFFF);
    zero_cluster(nc);
    return nc;
}

// ─────────────────────────────────────────────────────────────────────────────
// DIRECTORY WALKER  (static)
// ─────────────────────────────────────────────────────────────────────────────

typedef int (*dir_walk_fn)(struct dir_entry *e, uint32_t sector, int idx, void *ud);

static int walk_dir(uint32_t dir_cluster, dir_walk_fn fn, void *ud) {
    uint32_t cluster = dir_cluster;
    while (cluster < 0x0FFFFFF8) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint32_t s = 0; s < fs.sectors_per_cluster; s++) {
            uint8_t buf[512];
            read_sector(sector + s, buf);
            struct dir_entry *entries = (struct dir_entry *)buf;
            for (int i = 0; i < 16; i++)
                if (fn(&entries[i], sector + s, i, ud))
                    return 1;
        }
        cluster = read_fat_entry(cluster);
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FIND FILE IN DIRECTORY  (static)
// ─────────────────────────────────────────────────────────────────────────────

typedef struct { const char *name; uint32_t start_cluster; uint32_t size; uint8_t attributes; } find_ud;

static int find_cb(struct dir_entry *e, uint32_t sec, int idx, void *ud) {
    (void)sec; (void)idx;
    if (e->name[0] == 0x00) return 1;
    if (e->name[0] == 0xE5) return 0;
    if (e->attributes == 0x0F) return 0;

    find_ud *f = (find_ud *)ud;
    char fname[12];
    for (int j = 0; j < 8; j++) fname[j] = e->name[j];
    fname[8] = e->ext[0]; fname[9] = e->ext[1]; fname[10] = e->ext[2]; fname[11] = '\0';

    if (match_83(f->name, fname)) {
        f->start_cluster = ((uint32_t)e->cluster_high << 16) | e->cluster_low;
        f->size          = e->size;
        f->attributes    = e->attributes;
        return 1;
    }
    return 0;
}

static find_ud find_in(uint32_t dir_cluster, const char *name) {
    find_ud ud = { name, 0, 0, 0 };
    walk_dir(dir_cluster, find_cb, &ud);
    return ud;
}

// ─────────────────────────────────────────────────────────────────────────────
// PATH RESOLVER  (static)
// ─────────────────────────────────────────────────────────────────────────────

static uint32_t resolve_path(const char *path) {
    uint32_t cluster = (path[0] == '/') ? fs.root_cluster : cwd_cluster;
    if (path[0] == '/') path++;

    char component[13];
    while (*path) {
        int len = 0;
        while (path[len] && path[len] != '/') len++;
        if (len == 0) { path++; continue; }
        if (len > 12) return 0;
        for (int i = 0; i < len; i++) component[i] = path[i];
        component[len] = '\0';
        path += len;
        if (*path == '/') path++;

        if (str_cmp(component, ".") == 0) continue;
        if (str_cmp(component, "..") == 0) {
            find_ud fi = find_in(cluster, "..");
            cluster = fi.start_cluster ? fi.start_cluster : fs.root_cluster;
            continue;
        }

        find_ud fi = find_in(cluster, component);
        if (!fi.start_cluster) return 0;
        cluster = fi.start_cluster;
    }
    return cluster;
}

// ─────────────────────────────────────────────────────────────────────────────
// DIR ENTRY WRITER  (static)
// ─────────────────────────────────────────────────────────────────────────────

static int add_dir_entry(uint32_t dir_cluster, struct dir_entry *new_entry) {
    uint32_t cluster = dir_cluster;
    uint32_t prev    = 0;

    while (cluster < 0x0FFFFFF8) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint32_t s = 0; s < fs.sectors_per_cluster; s++) {
            uint8_t buf[512];
            read_sector(sector + s, buf);
            struct dir_entry *entries = (struct dir_entry *)buf;
            for (int i = 0; i < 16; i++) {
                if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
                    mem_cpy(&entries[i], new_entry, sizeof(struct dir_entry));
                    write_sector(sector + s, buf);
                    return 0;
                }
            }
        }
        prev    = cluster;
        cluster = read_fat_entry(cluster);
    }

    uint32_t nc = extend_chain(prev);
    if (!nc) return -1;
    uint8_t buf[512];
    mem_set(buf, 0, 512);
    mem_cpy((struct dir_entry *)buf, new_entry, sizeof(struct dir_entry));
    write_sector(cluster_to_sector(nc), buf);
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// DIR ENTRY UPDATER  (static) — updates size and/or start cluster in-place
// ─────────────────────────────────────────────────────────────────────────────

typedef struct { const char *name; uint32_t new_size; uint32_t new_cluster; int done; } update_ud;

static int update_entry_cb(struct dir_entry *e, uint32_t sector, int idx, void *ud) {
    if (e->name[0] == 0x00) return 1;
    if (e->name[0] == 0xE5) return 0;
    if (e->attributes == 0x0F) return 0;

    update_ud *u = (update_ud *)ud;
    char fname[12];
    for (int j = 0; j < 8; j++) fname[j] = e->name[j];
    fname[8] = e->ext[0]; fname[9] = e->ext[1]; fname[10] = e->ext[2]; fname[11] = '\0';
    if (!match_83(u->name, fname)) return 0;

    uint8_t buf[512];
    read_sector(sector, buf);
    struct dir_entry *entry = (struct dir_entry *)(buf + idx * sizeof(struct dir_entry));
    entry->size = u->new_size;
    if (u->new_cluster) {
        entry->cluster_high = (u->new_cluster >> 16) & 0xFFFF;
        entry->cluster_low  =  u->new_cluster        & 0xFFFF;
    }
    write_sector(sector, buf);
    u->done = 1;
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// UNLINK CALLBACK  (static, shared by unlink and rmdir)
// ─────────────────────────────────────────────────────────────────────────────

typedef struct { const char *name; int done; } unlink_ud;

static int unlink_cb(struct dir_entry *e, uint32_t sector, int idx, void *ud) {
    if (e->name[0] == 0x00) return 1;
    if (e->name[0] == 0xE5) return 0;
    if (e->attributes == 0x0F) return 0;

    unlink_ud *u = (unlink_ud *)ud;
    char fname[12];
    for (int j = 0; j < 8; j++) fname[j] = e->name[j];
    fname[8] = e->ext[0]; fname[9] = e->ext[1]; fname[10] = e->ext[2]; fname[11] = '\0';
    if (!match_83(u->name, fname)) return 0;

    uint32_t cluster = ((uint32_t)e->cluster_high << 16) | e->cluster_low;
    free_cluster_chain(cluster);

    uint8_t buf[512];
    read_sector(sector, buf);
    buf[idx * sizeof(struct dir_entry)] = 0xE5;
    write_sector(sector, buf);

    u->done = 1;
    return 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// FS INIT  (public)
// ─────────────────────────────────────────────────────────────────────────────

void fs_init(void) {
    uint8_t buf[512];
    read_sector(0, buf);
    struct bpb *b = (struct bpb *)buf;
    if (b->bytes_per_sector == 0) { kprintf("fs_init: invalid BPB\n"); return; }

    fs.bytes_per_sector    = b->bytes_per_sector;
    fs.sectors_per_cluster = b->sectors_per_cluster;
    fs.fat_start           = b->reserved_sectors;
    fs.data_start          = fs.fat_start + (b->fat_count * b->sectors_per_fat_32);
    fs.root_cluster        = b->root_cluster;
    fs.fat_size            = b->sectors_per_fat_32;
    fs.total_clusters      = (b->total_sectors_32 - fs.data_start) / fs.sectors_per_cluster;
    cwd_cluster            = fs.root_cluster;

    kprintf("fs initialized\n");
}

// ─────────────────────────────────────────────────────────────────────────────
// 1. cat <file>
// ─────────────────────────────────────────────────────────────────────────────

void fs_cat(const char *name) {
    find_ud fi = find_in(cwd_cluster, name);
    if (!fi.start_cluster) { kprintf("cat: '%s' not found\n", name); return; }

    uint32_t cluster = fi.start_cluster;
    uint32_t printed = 0;

    while (cluster < 0x0FFFFFF8 && printed < fi.size) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint32_t s = 0; s < fs.sectors_per_cluster && printed < fi.size; s++) {
            uint8_t buf[512];
            read_sector(sector + s, buf);
            uint32_t chunk = 512;
            if (printed + chunk > fi.size) chunk = fi.size - printed;
            for (uint32_t c = 0; c < chunk; c++) vga_putchar(buf[c]);
            printed += chunk;
        }
        cluster = read_fat_entry(cluster);
    }
    vga_putchar('\n');
}

// ─────────────────────────────────────────────────────────────────────────────
// 2. touch <file>  (fs_create)
// ─────────────────────────────────────────────────────────────────────────────

int fs_create(const char *name) {
    find_ud existing = find_in(cwd_cluster, name);
    if (existing.start_cluster) return 0;   // already exists, no-op like touch

    uint32_t nc = alloc_cluster();
    if (!nc) { kprintf("touch: disk full\n"); return -1; }
    zero_cluster(nc);

    struct dir_entry entry;
    mem_set(&entry, 0, sizeof(entry));
    char fat83[11];
    to_83(name, fat83);
    for (int i = 0; i < 8; i++) entry.name[i] = fat83[i];
    for (int i = 0; i < 3; i++) entry.ext[i]  = fat83[8 + i];
    entry.attributes   = 0x20;
    entry.cluster_high = (nc >> 16) & 0xFFFF;
    entry.cluster_low  =  nc        & 0xFFFF;
    entry.size         = 0;

    if (add_dir_entry(cwd_cluster, &entry) < 0) {
        free_cluster_chain(nc);
        return -1;
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 3. rm <file>  (fs_unlink)
// ─────────────────────────────────────────────────────────────────────────────

int fs_unlink(const char *name) {
    unlink_ud ud = { name, 0 };
    walk_dir(cwd_cluster, unlink_cb, &ud);
    if (!ud.done) { kprintf("rm: '%s' not found\n", name); return -1; }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. mv <src> <dst>  (fs_rename)
// ─────────────────────────────────────────────────────────────────────────────

typedef struct { const char *old_name; char new_fat83[11]; int done; } rename_ud;

static int rename_cb(struct dir_entry *e, uint32_t sector, int idx, void *ud) {
    if (e->name[0] == 0x00) return 1;
    if (e->name[0] == 0xE5) return 0;
    if (e->attributes == 0x0F) return 0;

    rename_ud *r = (rename_ud *)ud;
    char fname[12];
    for (int j = 0; j < 8; j++) fname[j] = e->name[j];
    fname[8] = e->ext[0]; fname[9] = e->ext[1]; fname[10] = e->ext[2]; fname[11] = '\0';
    if (!match_83(r->old_name, fname)) return 0;

    uint8_t buf[512];
    read_sector(sector, buf);
    struct dir_entry *entry = (struct dir_entry *)(buf + idx * sizeof(struct dir_entry));
    for (int i = 0; i < 8; i++) entry->name[i] = r->new_fat83[i];
    for (int i = 0; i < 3; i++) entry->ext[i]  = r->new_fat83[8 + i];
    write_sector(sector, buf);

    r->done = 1;
    return 1;
}

int fs_rename(const char *old_name, const char *new_name) {
    find_ud check = find_in(cwd_cluster, new_name);
    if (check.start_cluster) { kprintf("mv: '%s' already exists\n", new_name); return -1; }

    rename_ud ud;
    ud.old_name = old_name;
    ud.done     = 0;
    to_83(new_name, ud.new_fat83);

    walk_dir(cwd_cluster, rename_cb, &ud);
    if (!ud.done) { kprintf("mv: '%s' not found\n", old_name); return -1; }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. cp <src> <dst>
// ─────────────────────────────────────────────────────────────────────────────

int fs_cp(const char *src, const char *dst) {
    find_ud fi = find_in(cwd_cluster, src);
    if (!fi.start_cluster) { kprintf("cp: '%s' not found\n", src); return -1; }
    if (fi.attributes & 0x10) { kprintf("cp: '%s' is a directory\n", src); return -1; }

    if (fs_create(dst) < 0) return -1;

    uint32_t cluster = fi.start_cluster;
    uint32_t offset  = 0;

    while (cluster < 0x0FFFFFF8 && offset < fi.size) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint32_t s = 0; s < fs.sectors_per_cluster && offset < fi.size; s++) {
            uint8_t buf[512];
            read_sector(sector + s, buf);
            uint32_t chunk = 512;
            if (offset + chunk > fi.size) chunk = fi.size - offset;
            fs_write_file(dst, buf, offset, chunk);
            offset += chunk;
        }
        cluster = read_fat_entry(cluster);
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 6. ls [cluster]  (ls_dir)
// ─────────────────────────────────────────────────────────────────────────────

static int ls_cb(struct dir_entry *e, uint32_t sec, int idx, void *ud) {
    (void)sec; (void)idx; (void)ud;
    if (e->name[0] == 0x00) return 1;
    if (e->name[0] == 0xE5) return 0;
    if (e->attributes == 0x0F) return 0;

    uint32_t cluster = ((uint32_t)e->cluster_high << 16) | e->cluster_low;
    if (e->attributes & 0x10)
        kprintf("DIR   %.8s           cluster=%d\n", e->name, cluster);
    else
        kprintf("FILE  %.8s.%.3s  size=%-8d cluster=%d\n",
                e->name, e->ext, e->size, cluster);
    return 0;
}

void ls_dir(uint32_t dir_cluster) {
    if (dir_cluster == 0) dir_cluster = cwd_cluster;  
    walk_dir(dir_cluster, ls_cb, NULL);
}
// ─────────────────────────────────────────────────────────────────────────────
// 7. mkdir <name>
// ─────────────────────────────────────────────────────────────────────────────

int fs_mkdir(const char *name) {
    find_ud existing = find_in(cwd_cluster, name);
    if (existing.start_cluster) { kprintf("mkdir: '%s' exists\n", name); return -1; }

    uint32_t nc = alloc_cluster();
    if (!nc) { kprintf("mkdir: disk full\n"); return -1; }
    zero_cluster(nc);

    struct dir_entry entry;
    mem_set(&entry, 0, sizeof(entry));
    char fat83[11];
    to_83(name, fat83);
    for (int i = 0; i < 8; i++) entry.name[i] = fat83[i];
    for (int i = 0; i < 3; i++) entry.ext[i]  = ' ';
    entry.attributes   = 0x10;
    entry.cluster_high = (nc >> 16) & 0xFFFF;
    entry.cluster_low  =  nc        & 0xFFFF;
    entry.size         = 0;

    if (add_dir_entry(cwd_cluster, &entry) < 0) {
        free_cluster_chain(nc);
        return -1;
    }

    // write '.' and '..' inside new dir
    uint8_t buf[512];
    mem_set(buf, 0, 512);
    struct dir_entry *dot = (struct dir_entry *)buf;

    mem_set(dot[0].name, ' ', 8); mem_set(dot[0].ext, ' ', 3);
    dot[0].name[0]      = '.';
    dot[0].attributes   = 0x10;
    dot[0].cluster_high = (nc >> 16) & 0xFFFF;
    dot[0].cluster_low  =  nc        & 0xFFFF;

    mem_set(dot[1].name, ' ', 8); mem_set(dot[1].ext, ' ', 3);
    dot[1].name[0]      = '.'; dot[1].name[1] = '.';
    dot[1].attributes   = 0x10;
    dot[1].cluster_high = (cwd_cluster >> 16) & 0xFFFF;
    dot[1].cluster_low  =  cwd_cluster        & 0xFFFF;

    write_sector(cluster_to_sector(nc), buf);
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 8. mkdir -p <path>
// ─────────────────────────────────────────────────────────────────────────────

int fs_mkdir_p(const char *path) {
    char tmp[FS_MAX_PATH];
    str_cpy(tmp, path);
    char *p = tmp;
    if (*p == '/') p++;

    while (*p) {
        while (*p && *p != '/') p++;
        char save = *p;
        *p = '\0';
        if (!resolve_path(tmp))
            if (fs_mkdir(tmp) < 0) return -1;
        if (!save) break;
        *p = save;
        p++;
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 9. rmdir <name>
// ─────────────────────────────────────────────────────────────────────────────

static int dir_empty_cb(struct dir_entry *e, uint32_t sec, int idx, void *ud) {
    (void)sec; (void)idx;
    if (e->name[0] == 0x00) return 1;
    if (e->name[0] == 0xE5) return 0;
    if (e->name[0] == '.')  return 0;
    *(int *)ud = 0;
    return 1;
}

int fs_rmdir(const char *name) {
    find_ud fi = find_in(cwd_cluster, name);
    if (!fi.start_cluster) { kprintf("rmdir: '%s' not found\n", name); return -1; }
    if (!(fi.attributes & 0x10)) { kprintf("rmdir: '%s' not a directory\n", name); return -1; }

    int empty = 1;
    walk_dir(fi.start_cluster, dir_empty_cb, &empty);
    if (!empty) { kprintf("rmdir: '%s' not empty\n", name); return -1; }

    unlink_ud ud = { name, 0 };
    walk_dir(cwd_cluster, unlink_cb, &ud);
    return ud.done ? 0 : -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// 10. rmdir -r <name>
// ─────────────────────────────────────────────────────────────────────────────

int fs_rmdir_r(const char *name) {
    find_ud fi = find_in(cwd_cluster, name);
    if (!fi.start_cluster || !(fi.attributes & 0x10)) {
        kprintf("rmdir -r: '%s' not found or not a dir\n", name);
        return -1;
    }

    uint32_t saved = cwd_cluster;
    cwd_cluster = fi.start_cluster;

    uint32_t cluster = fi.start_cluster;
    while (cluster < 0x0FFFFFF8) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint32_t s = 0; s < fs.sectors_per_cluster; s++) {
            uint8_t buf[512];
            read_sector(sector + s, buf);
            struct dir_entry *entries = (struct dir_entry *)buf;
            for (int i = 0; i < 16; i++) {
                if (entries[i].name[0] == 0x00) goto next_cluster;
                if (entries[i].name[0] == 0xE5) continue;
                if (entries[i].name[0] == '.')  continue;

                char child[13];
                for (int j = 0; j < 8; j++) child[j] = entries[i].name[j];
                child[8] = entries[i].ext[0]; child[9] = entries[i].ext[1];
                child[10] = entries[i].ext[2]; child[11] = '\0';

                if (entries[i].attributes & 0x10) fs_rmdir_r(child);
                else fs_unlink(child);
            }
        }
        next_cluster:
        cluster = read_fat_entry(cluster);
    }

    cwd_cluster = saved;
    return fs_rmdir(name);
}

// ─────────────────────────────────────────────────────────────────────────────
// 11. cd <path>
// ─────────────────────────────────────────────────────────────────────────────

int fs_chdir(const char *path) {
    if (str_cmp(path, "/") == 0) {
        cwd_cluster = fs.root_cluster;
        str_cpy(cwd_path, "/");
        return 0;
    }

    uint32_t nc = resolve_path(path);
    if (!nc) { kprintf("cd: '%s' not found\n", path); return -1; }
    cwd_cluster = nc;

    if (path[0] == '/') {
        str_cpy(cwd_path, path);
    } else {
        if (cwd_path[str_len(cwd_path) - 1] != '/') str_cat(cwd_path, "/");
        str_cat(cwd_path, path);
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 12. pwd
// ─────────────────────────────────────────────────────────────────────────────

const char *fs_getcwd(void) {
    return cwd_path;
}

// ─────────────────────────────────────────────────────────────────────────────
// 13. stat <path>
// ─────────────────────────────────────────────────────────────────────────────

int fs_stat(const char *path, struct stat_info *out) {
    char tmp[FS_MAX_PATH];
    str_cpy(tmp, path);

    int last_slash = -1;
    for (int i = str_len(tmp) - 1; i >= 0; i--) {
        if (tmp[i] == '/') { last_slash = i; break; }
    }

    const char *base;
    uint32_t    dir_clust;

    if (last_slash < 0) {
        base      = path;
        dir_clust = cwd_cluster;
    } else {
        tmp[last_slash] = '\0';
        base      = path + last_slash + 1;
        dir_clust = resolve_path(tmp[0] ? tmp : "/");
        if (!dir_clust) return -1;
    }

    find_ud fi = find_in(dir_clust, base);
    if (!fi.start_cluster) return -1;

    out->size          = fi.size;
    out->start_cluster = fi.start_cluster;
    out->attributes    = fi.attributes;
    out->is_dir        = (fi.attributes & 0x10) != 0;
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// 14. find <name>
// ─────────────────────────────────────────────────────────────────────────────

static void find_recursive(uint32_t dir_cluster, const char *target, const char *cur_path) {
    uint32_t cluster = dir_cluster;
    while (cluster < 0x0FFFFFF8) {
        uint32_t sector = cluster_to_sector(cluster);
        for (uint32_t s = 0; s < fs.sectors_per_cluster; s++) {
            uint8_t buf[512];
            read_sector(sector + s, buf);
            struct dir_entry *entries = (struct dir_entry *)buf;
            for (int i = 0; i < 16; i++) {
                if (entries[i].name[0] == 0x00) return;
                if (entries[i].name[0] == 0xE5) continue;
                if (entries[i].name[0] == '.')  continue;
                if (entries[i].attributes == 0x0F) continue;

                char child[13];
                for (int j = 0; j < 8; j++) child[j] = entries[i].name[j];
                child[8] = entries[i].ext[0]; child[9] = entries[i].ext[1];
                child[10] = entries[i].ext[2]; child[11] = '\0';

                char full[FS_MAX_PATH];
                str_cpy(full, cur_path);
                if (full[str_len(full) - 1] != '/') str_cat(full, "/");
                str_cat(full, child);

                if (match_83(target, (char *)entries[i].name))
                    kprintf("%s\n", full);

                if (entries[i].attributes & 0x10) {
                    uint32_t sub = ((uint32_t)entries[i].cluster_high << 16)
                                 | entries[i].cluster_low;
                    find_recursive(sub, target, full);
                }
            }
        }
        cluster = read_fat_entry(cluster);
    }
}

void fs_find(const char *name) {
    find_recursive(cwd_cluster, name, cwd_path);
}

// ─────────────────────────────────────────────────────────────────────────────
// 15. df
// ─────────────────────────────────────────────────────────────────────────────

void fs_df(void) {
    uint32_t free_clusters = 0;
    for (uint32_t c = 2; c < fs.total_clusters + 2; c++)
        if (read_fat_entry(c) == 0x00000000) free_clusters++;

    uint32_t csz     = fs.sectors_per_cluster * 512;
    uint32_t total_kb = (fs.total_clusters * csz) / 1024;
    uint32_t free_kb  = (free_clusters     * csz) / 1024;

    kprintf("Total : %d KB\n", total_kb);
    kprintf("Used  : %d KB\n", total_kb - free_kb);
    kprintf("Free  : %d KB\n", free_kb);
}

// ─────────────────────────────────────────────────────────────────────────────
// 16. du <file>
// ─────────────────────────────────────────────────────────────────────────────

void fs_du(const char *name) {
    find_ud fi = find_in(cwd_cluster, name);
    if (!fi.start_cluster) { kprintf("du: '%s' not found\n", name); return; }

    uint32_t count = 0, cluster = fi.start_cluster;
    while (cluster < 0x0FFFFFF8 && cluster >= 2) {
        count++;
        cluster = read_fat_entry(cluster);
    }

    uint32_t csz = fs.sectors_per_cluster * 512;
    kprintf("'%s'  %d bytes  %d clusters  %d KB\n",
            name, fi.size, count, (count * csz) / 1024);
}

// ─────────────────────────────────────────────────────────────────────────────
// 17. sync
// ─────────────────────────────────────────────────────────────────────────────

void fs_sync(void) {
    outb(0x1F7, 0xE7);
    while (inb(0x1F7) & 0x80);
}

// ─────────────────────────────────────────────────────────────────────────────
// 18. echo <text> [> file]
//     filename == NULL  → print to terminal
//     filename != NULL  → write text to file (create if needed)
// ─────────────────────────────────────────────────────────────────────────────

int fs_echo(const char *text, const char *filename) {
    if (!filename) {
        kprintf("%s\n", text);
        return 0;
    }
    uint32_t len = (uint32_t)str_len(text);
    return fs_write_file(filename, (const uint8_t *)text, 0, len);
}

// ─────────────────────────────────────────────────────────────────────────────
// READ / WRITE / TRUNCATE  (needed by cp and echo internally + direct syscalls)
// ─────────────────────────────────────────────────────────────────────────────

int fs_read_file(const char *name, uint8_t *buffer, uint32_t offset, uint32_t len) {
    find_ud fi = find_in(cwd_cluster, name);
    if (!fi.start_cluster) { kprintf("read: '%s' not found\n", name); return -1; }
    if (offset >= fi.size) return 0;
    if (offset + len > fi.size) len = fi.size - offset;

    uint32_t cluster      = fi.start_cluster;
    uint32_t bytes_ps     = fs.sectors_per_cluster * 512;
    uint32_t clust_idx    = offset / bytes_ps;
    uint32_t clust_offset = offset % bytes_ps;

    for (uint32_t i = 0; i < clust_idx && cluster < 0x0FFFFFF8; i++)
        cluster = read_fat_entry(cluster);

    uint32_t bytes_read = 0;
    while (bytes_read < len && cluster < 0x0FFFFFF8) {
        uint32_t sector  = cluster_to_sector(cluster);
        uint32_t sec_idx = clust_offset / 512;
        uint32_t sec_off = clust_offset % 512;

        for (; sec_idx < fs.sectors_per_cluster && bytes_read < len; sec_idx++) {
            uint8_t buf[512];
            read_sector(sector + sec_idx, buf);
            uint32_t chunk = 512 - sec_off;
            if (chunk > len - bytes_read) chunk = len - bytes_read;
            mem_cpy(buffer + bytes_read, buf + sec_off, chunk);
            bytes_read += chunk;
            sec_off = 0;
        }
        clust_offset = 0;
        cluster = read_fat_entry(cluster);
    }
    return (int)bytes_read;
}

int fs_write_file(const char *name, const uint8_t *data, uint32_t offset, uint32_t len) {
    find_ud fi = find_in(cwd_cluster, name);
    if (!fi.start_cluster) {
        if (fs_create(name) < 0) return -1;
        fi = find_in(cwd_cluster, name);
    }

    uint32_t bytes_ps  = fs.sectors_per_cluster * 512;
    uint32_t needed    = offset + len;
    uint32_t cluster   = fi.start_cluster;
    uint32_t prev      = 0;
    uint32_t clust_idx = 0;

    while (clust_idx * bytes_ps < needed) {
        if (cluster >= 0x0FFFFFF8 || cluster == 0) {
            uint32_t nc = prev ? extend_chain(prev) : alloc_cluster();
            if (!nc) { kprintf("write: disk full\n"); return -1; }
            if (!fi.start_cluster) {
                fi.start_cluster = nc;
                update_ud ud = { name, 0, nc, 0 };
                walk_dir(cwd_cluster, update_entry_cb, &ud);
            }
            cluster = nc;
        }

        uint32_t clust_start = clust_idx * bytes_ps;
        uint32_t clust_end   = clust_start + bytes_ps;

        if (offset < clust_end && offset + len > clust_start) {
            uint32_t sector    = cluster_to_sector(cluster);
            uint32_t local_off = (offset > clust_start) ? offset - clust_start : 0;
            uint32_t data_off  = (offset > clust_start) ? 0 : clust_start - offset;

            for (uint32_t s = local_off / 512; s < fs.sectors_per_cluster; s++) {
                uint32_t sec_off  = (s == local_off / 512) ? local_off % 512 : 0;
                uint32_t file_pos = clust_start + s * 512 + sec_off;
                if (file_pos >= offset + len) break;

                uint8_t buf[512];
                read_sector(sector + s, buf);
                uint32_t chunk = 512 - sec_off;
                uint32_t remaining = (offset + len) - file_pos;
                if (chunk > remaining) chunk = remaining;
                mem_cpy(buf + sec_off, data + data_off, chunk);
                write_sector(sector + s, buf);
                data_off += chunk;
            }
        }

        prev = cluster;
        cluster = read_fat_entry(cluster);
        clust_idx++;
    }

    uint32_t new_size = (needed > fi.size) ? needed : fi.size;
    update_ud ud = { name, new_size, 0, 0 };
    walk_dir(cwd_cluster, update_entry_cb, &ud);
    return (int)len;
}

int fs_truncate(const char *name, uint32_t new_size) {
    find_ud fi = find_in(cwd_cluster, name);
    if (!fi.start_cluster) { kprintf("truncate: '%s' not found\n", name); return -1; }

    uint32_t bytes_ps      = fs.sectors_per_cluster * 512;
    uint32_t clusters_need = (new_size + bytes_ps - 1) / bytes_ps;
    if (clusters_need == 0) clusters_need = 1;

    uint32_t cluster = fi.start_cluster;
    uint32_t count   = 0;

    while (count + 1 < clusters_need && cluster < 0x0FFFFFF8) {
        cluster = read_fat_entry(cluster);
        count++;
    }

    if (cluster < 0x0FFFFFF8) {
        uint32_t tail = read_fat_entry(cluster);
        write_fat_entry(cluster, 0x0FFFFFFF);
        free_cluster_chain(tail);
    }

    update_ud ud = { name, new_size, 0, 0 };
    walk_dir(cwd_cluster, update_entry_cb, &ud);
    return 0;
}