// syscalls.h
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

struct stat_info {
    uint32_t size;
    uint32_t start_cluster;
    uint8_t  attributes;
    int      is_dir;
};

// ====================== I/O syscalls ======================

void     write(const uint8_t *str);
void     read(uint8_t *buffer, uint32_t len);

// ====================== File System related syscalls ======================

void     cat(uint8_t *name);
void     touch(uint8_t *name);
void     rm(uint8_t *name);
void     mv(uint8_t *src, uint8_t *dst);
void     cp(uint8_t *src, uint8_t *dst);
void     ls(uint32_t dir_cluster);
void     mkdir(uint8_t *name);
void     mkdir_p(uint8_t *path);
void     rmdir(uint8_t *name);
void     rmdir_r(uint8_t *name);
void     cd(uint8_t *path);
uint8_t *pwd(void);
void     stat(uint8_t *path, struct stat_info *info);
void     find(uint8_t *name);
void     df(void);
void     du(uint8_t *name);
void     sync(void);
void     echo(uint8_t *text, uint8_t *filename);
void     clear(void);

#endif