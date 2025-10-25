# Toolchain
CC = i686-linux-gnu-gcc
LD = i686-linux-gnu-ld
NASM = nasm
OBJCOPY = i686-linux-gnu-objcopy

# Directories
SRC_DIR = src
OUT_DIR = build/i686
INCLUDE_DIR = include
BOOT_SRC = boot.asm
LINKER_SCRIPT = linker.ld

# Output files
BOOT_BIN = $(OUT_DIR)/boot.bin
KERNEL_ELF = $(OUT_DIR)/kernel.elf
KERNEL_BIN = $(OUT_DIR)/kernel.bin
IMG = $(OUT_DIR)/kernel.img

# C sources and objects
C_SRCS = $(SRC_DIR)/kmain.c $(filter-out $(SRC_DIR)/kmain.c, $(wildcard $(SRC_DIR)/*.c))
C_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OUT_DIR)/%.o,$(C_SRCS))

# Compiler flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra -I$(INCLUDE_DIR) -ffunction-sections
LDFLAGS = -T $(LINKER_SCRIPT) -nostdlib

# Default target
all: $(IMG)

# -------------------------------
# Build bootloader
# -------------------------------
$(BOOT_BIN): $(BOOT_SRC)
	mkdir -p $(OUT_DIR)
	$(NASM) -f bin $< -o $@

# -------------------------------
# Compile C sources
# -------------------------------
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# -------------------------------
# Link kernel ELF
# -------------------------------
$(KERNEL_ELF): $(C_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# -------------------------------
# Convert ELF to binary
# -------------------------------
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# -------------------------------
# Create bootable image
# -------------------------------
$(IMG): $(BOOT_BIN) $(KERNEL_BIN)
	# 2 MB blank image
	dd if=/dev/zero of=$@ bs=512 count=4096 
	# Write bootloader first sector
	dd if=$(BOOT_BIN) of=$@ conv=notrunc 
	# Write kernel binary after first sector
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc 
	@echo "[*] Bootable image created: $@"

# -------------------------------
# Run in QEMU
# -------------------------------
run: $(IMG)
	qemu-system-i386 -drive format=raw,file=$(IMG) --no-shutdown --no-reboot 

# -------------------------------
# Run and Debug
# -------------------------------
debug: $(IMG)
	qemu-system-i386 -drive format=raw,file=$(IMG) --no-shutdown --no-reboot -s -S

# -------------------------------
# Clean build
# -------------------------------

clean:
	rm -rf $(OUT_DIR)
