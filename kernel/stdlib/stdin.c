#include "stdio.h"

int gets(char *buf, int max) {
    // security checks before syscall

    // null pointer check
    if (!buf)
        return -1;

    // max length sanity check
    if (max <= 0 || max > 4096)
        return -1;

    // check buffer is not in kernel space
    // (adjust this address to your kernel boundary)
    if ((uint32_t)buf >= 0xC0000000)
        return -1;

    // all checks passed — fire syscall
    read((uint8_t*)buf, (uint32_t)max);

    return 0;
}