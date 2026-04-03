# Directories
BOOT_DIR    := boot
KERNEL_DIR  := kernel
OBJ_DIR     := obj

# Tools
CC      := i386-elf-gcc
LD      := i386-elf-ld
AS      := nasm
OBJCOPY := objcopy

# Flags
CFLAGS  := -ffreestanding -m32 -O0 -Wall -Wextra
LDFLAGS := -m elf_i386 -T linker.ld

# Files
KERNEL_C        := $(KERNEL_DIR)/main.c
KERNEL_OBJ      := $(OBJ_DIR)/main.o



BOOTLOADER_ASM  := $(BOOT_DIR)/stage1.asm
BOOTLOADER_BIN  := $(OBJ_DIR)/stage1.bin

KERNEL_ELF      := $(OBJ_DIR)/kernel.elf
KERNEL_RAW      := $(OBJ_DIR)/kernel.raw

OS_IMAGE        := $(OBJ_DIR)/os.img

# Default target
all: $(OS_IMAGE)

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile C kernel
$(KERNEL_OBJ): $(KERNEL_C) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble kernel entry (ELF)
$(ENTRY_OBJ): $(ENTRY_ASM) | $(OBJ_DIR)
	$(AS) -f elf32 $< -o $@

# Link kernel
$(KERNEL_ELF): $(KERNEL_OBJ) $(ENTRY_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# Convert ELF → raw binary
$(KERNEL_RAW): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Build OS image
$(OS_IMAGE): $(KERNEL_RAW)
	$(eval KERNEL_SECTORS := $(shell python3 -c "import math, os; print(max(1, math.ceil(os.path.getsize('$(KERNEL_RAW)') / 512)))"))
	$(AS) -f bin $(BOOTLOADER_ASM) -DKERNEL_SECTORS=$(KERNEL_SECTORS) -o $(BOOTLOADER_BIN)
	cat $(BOOTLOADER_BIN) $(KERNEL_RAW) > $(OS_IMAGE)

# Run in QEMU
run: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE)

# Clean
clean:
	rm -rf $(OBJ_DIR)
