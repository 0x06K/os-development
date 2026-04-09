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
CFLAGS  := -ffreestanding -m32 -O0 -Wall -Wextra -g3 -I$(KERNEL_DIR)
LDFLAGS := -m elf_i386 -T linker.ld

# Files
BL_ASM  := $(BOOT_DIR)/stage1.asm
BL_OBJ  := $(OBJ_DIR)/stage1.bin
KERNEL_ELF      := $(OBJ_DIR)/kernel.elf
KERNEL_RAW      := $(OBJ_DIR)/kernel.raw
OS_IMAGE        := $(OBJ_DIR)/os.img

# automatically finds all .c files in kernel/
SRCS       := $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_OBJ := $(patsubst $(KERNEL_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


# Default target
all: $(OS_IMAGE)

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# creates obj/vga/vga.o  obj/io/io.o etc — no collisions
$(OBJ_DIR)/%.o: $(KERNEL_DIR)/%.c | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# Assemble entry
$(BL_OBJ): $(BL_ASM) | $(OBJ_DIR)
	$(AS) -f bin $< -o $@

# Link kernel
$(KERNEL_ELF): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# Convert ELF → raw binary
$(KERNEL_RAW): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Build OS image
$(OS_IMAGE): $(KERNEL_RAW) | $(OBJ_DIR)
	$(eval KERNEL_SECTORS := $(shell python3 -c "import math, os; print(max(1, math.ceil(os.path.getsize('$(KERNEL_RAW)') / 512)))"))
	$(AS) -f bin $(BL_ASM) -DKERNEL_SECTORS=$(KERNEL_SECTORS) -o $(BL_OBJ)
	cat $(BL_OBJ) $(KERNEL_RAW) > $(OS_IMAGE)

# Run in QEMU
run: $(OS_IMAGE)
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE)

# Clean
clean:
	rm -rf $(OBJ_DIR)