#!/bin/bash

# Directories
OUT_DIR="build/i686"
mkdir -p $OUT_DIR

# File names
BOOT=boot.asm
KERNEL_C=src/kmain.c
KERNEL_ELF=$OUT_DIR/kernel.elf
KERNEL_BIN=$OUT_DIR/kernel.bin
IMG=$OUT_DIR/kernel.img
OBJ_C=$OUT_DIR/kernel_c.o

# -------------------------------
# Assemble bootloader (512 bytes)
# -------------------------------
nasm -f bin $BOOT -o $OUT_DIR/boot.bin

# -------------------------------
# Compile kernel C
# -------------------------------
i686-linux-gnu-gcc -ffreestanding -ffunction-sections -g -O2 -Wall -Wextra -c $KERNEL_C -o $OBJ_C -Iinclude

# -------------------------------
# Link kernel to ELF
# -------------------------------
i686-linux-gnu-ld -T linker.ld -nostdlib -o $KERNEL_ELF $OBJ_C

# -------------------------------
# Convert ELF to raw binary
# -------------------------------
i686-linux-gnu-objcopy -O binary $KERNEL_ELF $KERNEL_BIN

# -------------------------------
# Create bootable .img
# -------------------------------
# 2 MB blank image
dd if=/dev/zero of=$IMG bs=512 count=4096

# Write bootloader first
dd if=$OUT_DIR/boot.bin of=$IMG conv=notrunc

# Write kernel binary after 1 sector
dd if=$KERNEL_BIN of=$IMG bs=512 seek=1 conv=notrunc

echo "[*] Bootable raw image created: $IMG"
KERNEL_SECTORS=$(( ( $(stat -c%s $OUT_DIR/kernel.bin) + 511 ) / 512 ))
echo "Kernel size: $KERNEL_SECTORS sectors"

# -------------------------------
# Boot in QEMU
# -------------------------------
qemu-system-i386 -drive format=raw,file=$IMG --no-shutdown
#qemu-system-i386 -drive format=raw,file=$IMG --no-shutdown --no-reboot -s -S
