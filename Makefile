# ─────────────────────────────────────────
#  DIRECTORIES
# ─────────────────────────────────────────
BOOT_DIR   := boot
KERNEL_DIR := kernel
OBJ_DIR    := obj

# ─────────────────────────────────────────
#  TOOLS
# ─────────────────────────────────────────
CC      := i386-elf-gcc
LD      := i386-elf-ld
AS      := nasm
OBJCOPY := objcopy

# ─────────────────────────────────────────
#  FLAGS
# ─────────────────────────────────────────
CFLAGS  := -ffreestanding -m32 -O0 -Wall -Wextra -g3 -I$(KERNEL_DIR)
LDFLAGS := -m elf_i386 -T linker.ld

# ─────────────────────────────────────────
#  FILES
# ─────────────────────────────────────────
BL_ASM     := $(BOOT_DIR)/stage1.asm
BL_BIN     := $(OBJ_DIR)/stage1.bin
KERNEL_ELF := $(OBJ_DIR)/kernel.elf
KERNEL_RAW := $(OBJ_DIR)/kernel.raw
OS_IMAGE   := $(OBJ_DIR)/os.img

# ─────────────────────────────────────────
#  SOURCE DISCOVERY
# ─────────────────────────────────────────
SRCS       := $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_OBJ := $(patsubst $(KERNEL_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# ─────────────────────────────────────────
#  DEFAULT TARGET
# ─────────────────────────────────────────
all: $(OS_IMAGE)

# ─────────────────────────────────────────
#  CREATE OBJ DIRECTORY
# ─────────────────────────────────────────
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ─────────────────────────────────────────
#  COMPILE C FILES
# ─────────────────────────────────────────
$(OBJ_DIR)/%.o: $(KERNEL_DIR)/%.c | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ─────────────────────────────────────────
#  ASSEMBLE BOOTLOADER
# ─────────────────────────────────────────
$(BL_BIN): $(BL_ASM) | $(OBJ_DIR)
	$(AS) -f bin $< -o $@

# ─────────────────────────────────────────
#  LINK KERNEL
# ─────────────────────────────────────────
$(KERNEL_ELF): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# ─────────────────────────────────────────
#  ELF → RAW BINARY
# ─────────────────────────────────────────
$(KERNEL_RAW): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

compile: $(KERNEL_RAW) $(BL_BIN)

$(OS_IMAGE): $(BL_BIN) $(KERNEL_RAW)
	dd if=/dev/zero of=$(OS_IMAGE) bs=1M count=32
	mkfs.fat -F 32 $(OS_IMAGE)
	dd if=$(BL_BIN) of=$(OS_IMAGE) bs=1 count=3 conv=notrunc
	dd if=$(BL_BIN) of=$(OS_IMAGE) bs=1 skip=90 seek=90 count=356 conv=notrunc
	dd if=$(KERNEL_RAW) of=$(OS_IMAGE) bs=512 seek=1 conv=notrunc
	sudo mount -o loop $(OS_IMAGE) /mnt/os
	echo "hello from fat32" | sudo tee /mnt/os/test.txt
	sudo umount /mnt/os
# ─────────────────────────────────────────
#  RUN
# ─────────────────────────────────────────
run: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE)

# ─────────────────────────────────────────
#  CLEAN
# ─────────────────────────────────────────
clean:
	rm -rf $(OBJ_DIR)

.PHONY: all run clean