# ─────────────────────────────────────────
#  DIRECTORIES
# ─────────────────────────────────────────
USER_DIR    := .
STDLIB_DIR  := stdlib
SYSCALL_DIR := syscalls
OBJ_DIR     := obj

# ─────────────────────────────────────────
#  TOOLS
# ─────────────────────────────────────────
CC      := i386-elf-gcc
LD      := i386-elf-ld
OBJCOPY := objcopy

# ─────────────────────────────────────────
#  FLAGS
# ─────────────────────────────────────────
GCC_INCLUDES := $(shell i386-elf-gcc -print-file-name=include)

CFLAGS  := -ffreestanding -m32 -O0 -Wall -Wextra -g3 \
           -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
           -I. \
           -I$(GCC_INCLUDES)
LDFLAGS := -m elf_i386 -T user.ld

# ─────────────────────────────────────────
#  FILES
# ─────────────────────────────────────────
USER_ELF := $(OBJ_DIR)/user.elf
USER_RAW := $(OBJ_DIR)/user.bin

# ─────────────────────────────────────────
#  SOURCE DISCOVERY
# ─────────────────────────────────────────
SRCS     := $(shell find $(STDLIB_DIR) $(SYSCALL_DIR) -name "*.c") user.c
USER_OBJ := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

# ─────────────────────────────────────────
#  DEFAULT TARGET
# ─────────────────────────────────────────
all: $(USER_RAW)

# ─────────────────────────────────────────
#  CREATE OBJ DIRECTORY
# ─────────────────────────────────────────
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ─────────────────────────────────────────
#  COMPILE C FILES
# ─────────────────────────────────────────
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ─────────────────────────────────────────
#  LINK
# ─────────────────────────────────────────
$(USER_ELF): $(USER_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# ─────────────────────────────────────────
#  ELF → RAW BINARY
# ─────────────────────────────────────────
$(USER_RAW): $(USER_ELF)
	$(OBJCOPY) -O binary $< $@

# ─────────────────────────────────────────
#  CLEAN
# ─────────────────────────────────────────
clean:
	rm -rf $(OBJ_DIR)

.PHONY: all clean