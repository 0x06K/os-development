#!/bin/bash
set -e

# Config
ARCH="i686"
BUILD="build/$ARCH"
SRC="src"
INC="include"

# Source files
ASM_FILES=("boot.asm" "gdt.asm")
C_FILES=("kmain.c" "console.c" "gdt.c" "serial.c" "vga.c")


# Flags
CFLAGS="-ffreestanding -m32 -O2 -Wall -Wextra -I$INC -g"
LDFLAGS="-T link.ld -nostdlib -ffreestanding -Wl,--build-id=none -no-pie"

# Clean
echo "[1/4] Cleaning..."
rm -rf "$BUILD" && mkdir -p "$BUILD"

# Assemble
echo "[2/4] Assembling..."
for asm in "${ASM_FILES[@]}"; do
    base="${asm%.asm}"
    if [[ "$base" == "boot" ]]; then
        nasm -f bin "$SRC/$asm" -o "$BUILD/$base.bin"
    else
        nasm -f elf32 "$SRC/$asm" -o "$BUILD/${base}_flush.o"
    fi
done

# Compile
echo "[3/4] Compiling..."
for c in "${C_FILES[@]}"; do
    base="${c%.c}"
    i686-linux-gnu-gcc $CFLAGS -c "$SRC/$c" -o "$BUILD/$base.o"
done

# Link
i686-linux-gnu-gcc $LDFLAGS -o "$BUILD/kernel.elf" "$BUILD"/*.o
objcopy -O binary "$BUILD/kernel.elf" "$BUILD/kernel.bin"

# Create bootable image
echo "[4/4] Creating bootable image..."
cat "$BUILD/boot.bin" "$BUILD/kernel.bin" > "$BUILD/os.img"

echo "✅ Built: $BUILD/os.img"
echo "🐛 Debug symbols: $BUILD/kernel.elf"

# Run
[ "$1" = "run" ] && qemu-system-i386 -drive format=raw,file="build/i686/os.img" -s -S &

# Use with: gdb build/i686/kernel.elf -ex "target remote :1234"