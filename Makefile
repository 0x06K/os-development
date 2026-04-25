# ─────────────────────────────────────────
#  TOP-LEVEL MAKEFILE
#  ./
#  ├── ring0/   (kernel)
#  └── ring3/   (userspace)
# ─────────────────────────────────────────

RING0_DIR  := ring0
RING3_DIR  := ring3
OS_IMAGE   := $(RING0_DIR)/obj/os.img
USER_BIN   := $(RING3_DIR)/obj/user.bin
MOUNT_POINT := /mnt/os

# ─────────────────────────────────────────
#  DEFAULT — build everything
# ─────────────────────────────────────────
all: kernel user image

# ─────────────────────────────────────────
#  BUILD KERNEL
# ─────────────────────────────────────────
kernel:
	$(MAKE) -C $(RING0_DIR)

# ─────────────────────────────────────────
#  BUILD USERSPACE
# ─────────────────────────────────────────
user:
	$(MAKE) -C $(RING3_DIR)

# ─────────────────────────────────────────
#  COPY USER BINARY INTO OS IMAGE
# ─────────────────────────────────────────
image: $(OS_IMAGE) $(USER_BIN)
	sudo mkdir -p $(MOUNT_POINT)
	sudo mount -o loop $(OS_IMAGE) $(MOUNT_POINT)
	sudo cp $(USER_BIN) $(MOUNT_POINT)/
	sudo umount $(MOUNT_POINT)

# ─────────────────────────────────────────
#  RUN IN QEMU
# ─────────────────────────────────────────
run: all
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -m 4G

# ─────────────────────────────────────────
#  DEBUG — run with GDB stub
# ─────────────────────────────────────────
debug: all
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -s -S -no-reboot -no-shutdown -m 4G

# ─────────────────────────────────────────
#  CLEAN EVERYTHING
# ─────────────────────────────────────────
clean:
	$(MAKE) -C $(RING0_DIR) clean
	$(MAKE) -C $(RING3_DIR) clean

.PHONY: all kernel user image run debug clean