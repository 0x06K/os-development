#!/bin/bash
set -e

# -------------------------------
# Directories
# -------------------------------
OUT_DIR="build/i686"
mkdir -p "$OUT_DIR"

SRC_DIR="src"
BOOT="boot.asm"
KERNEL_C="$SRC_DIR/kmain.c"
LINKER="tools/build/linker.ld"

KERNEL_ELF="$OUT_DIR/kernel.elf"
KERNEL_BIN="$OUT_DIR/kernel.bin"
IMG="$OUT_DIR/kernel.img"
OBJ_C="$OUT_DIR/kernel_c.o"

# -------------------------------
# Assemble bootloader (512 bytes)
# -------------------------------
echo "[*] Assembling bootloader..."
nasm -f bin "$BOOT" -o "$OUT_DIR/boot.bin"

# -------------------------------
# Compile kernel C
# -------------------------------
echo "[*] Compiling kernel C..."
i686-linux-gnu-gcc -ffreestanding -ffunction-sections -g -O2 -Wall -Wextra \
    -c "$KERNEL_C" -o "$OBJ_C" -Iinclude

# -------------------------------
# Link kernel to ELF
# -------------------------------
echo "[*] Linking kernel..."
i686-linux-gnu-ld -T "$LINKER" -nostdlib -o "$KERNEL_ELF" "$OBJ_C"

# -------------------------------
# Convert ELF to raw binary
# -------------------------------
echo "[*] Converting ELF to raw binary..."
i686-linux-gnu-objcopy -O binary "$KERNEL_ELF" "$KERNEL_BIN"

# -------------------------------
# Create bootable image
# -------------------------------
echo "[*] Creating bootable image..."
# 2 MB blank image
dd if=/dev/zero of="$IMG" bs=512 count=4096 status=none

# Write bootloader first sector
dd if="$OUT_DIR/boot.bin" of="$IMG" conv=notrunc status=none

# Write kernel binary after first sector
dd if="$KERNEL_BIN" of="$IMG" bs=512 seek=1 conv=notrunc status=none

KERNEL_SECTORS=$(( ( $(stat -c%s "$KERNEL_BIN") + 511 ) / 512 ))
echo "[*] Bootable image created: $IMG"
echo "[*] Kernel size: $KERNEL_SECTORS sectors"

# -------------------------------
# Boot in QEMU
# -------------------------------
echo "[*] Booting in QEMU..."
qemu-system-i386 -drive format=raw,file="$IMG" --no-shutdown
# Alternative for debugging with GDB:
# qemu-system-i386 -drive format=raw,file="$IMG" --no-shutdown --no-reboot -s -S
