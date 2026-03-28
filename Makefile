all: os.img

kernel.o: kernel.c
	i386-elf-gcc -ffreestanding -m32 -O0 -c kernel.c -o kernel.o

kernel_entry.o: kernel_entry.asm
	nasm -f elf kernel_entry.asm -o kernel_entry.o

kernel.raw: kernel_entry.o kernel.o
	i386-elf-ld -m elf_i386 -T linker.ld -o kernel.elf kernel_entry.o kernel.o
	objcopy -O binary kernel.elf kernel.raw

os.img: kernel.raw
	$(eval KERNEL_SECTORS := $(shell python3 -c "import math, os; print(max(1, math.ceil(os.path.getsize('kernel.raw') / 512)))"))
	nasm -f bin bootloader.asm -DKERNEL_SECTORS=$(KERNEL_SECTORS) -o bootloader.bin
	cat bootloader.bin kernel.raw > os.img

run: os.img
	qemu-system-i386 -drive format=raw,file=os.img

clean:
	rm -f *.bin *.o *.img *.raw *.elf
