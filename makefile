.ONESHELL:

AS=as
CC=clang

CC_FLAGS+= -c -ffreestanding -target x86_64-none-elf -fno-integrated-as -mcmodel=large -g -nostdlib -mno-avx -mno-sse
ifeq ($(MAKECMDGOALS), test)
	CC_FLAGS += -D TEST=1	
endif

KERNEL_SRC_DIR := src/kernel/
BOOT_SRC_DIR := src/boot/
INCLUDE_DIR := src/include/

BOOT_SRCS := $(shell find $(BOOT_SRC_DIR) -name '*.S')
BOOT_OBJS := $(BOOT_SRCS:.S=.s.o)


# Setting up kernel src variables 
KERNELC_SRCS := $(shell find $(KERNEL_SRC_DIR) -name '*.c')
KERNELS_SRCS := $(shell find $(KERNEL_SRC_DIR) -name '*.S') 
KERNEL_OBJS := $(KERNELC_SRCS:.c=.c.o)
KERNEL_OBJS	+= $(KERNELS_SRCS:.S=.s.o)


BOOT_SECTOR_SIZE:=0
KERNEL_SECTOR_SIZE:=0

ISO := bin/os.bin
GET_SECTORS = $(shell echo $$( expr $$(stat -c%s $(1)) / 512 ))

all: boot run

%.s.o: %.S
	$(AS) -I$(INCLUDE_DIR) -o $@ -c $<


%.c.o: %.c	
	$(CC) -I$(INCLUDE_DIR) $(CC_FLAGS) -o $@ -c $<


test: run


run: iso
	qemu-system-x86_64 --vga std --no-reboot -d int $(ISO)


debug: iso
	qemu-system-x86_64 -s -S --vga std --no-reboot -d int $(ISO)

grub-debug: grub-iso
	qemu-system-x86_64 -s -S --vga std --no-reboot -d int $(ISO)

grub-run: grub-iso
	qemu-system-x86_64 --vga std --no-reboot -d int $(ISO)
bochs: iso
	truncate -s 2949120 $(ISO)
	bochs-gdb

usb: iso 
	./scripts/create_usb.sh

grub-usb: grub-iso
	./scripts/create_usb.sh

grub-iso: kernel
	$(MAKE) -C ./src/shell/
	mv src/shell/shell.elf iso/
	cd iso/
	tar cvf boot/initramfs.tar shell.elf file1.txt file2.txt
	cd ..
	mv bin/kernel.bin iso/boot/
	grub-mkrescue -o $(ISO) iso/

iso: boot kernel
	$(eval BOOT_SECTOR_SIZE=$(call GET_SECTORS,./bin/boot.bin))
	$(eval KERNEL_SECTOR_SIZE=$(call GET_SECTORS,./bin/kernel.bin))
	$(eval KERNEL_SECTOR_SIZE=$(shell echo $$(expr $(KERNEL_SECTOR_SIZE) + 1)))
	dd if=/dev/zero of=$(ISO) bs=2M count=1
	dd if=./bin/boot.bin of=$(ISO) conv=notrunc bs=512 seek=0 count=$(BOOT_SECTOR_SIZE) 
	dd if=./bin/kernel.bin of=$(ISO) conv=notrunc bs=512 seek=$(BOOT_SECTOR_SIZE) count=$(KERNEL_SECTOR_SIZE) 
	echo '2048,,L' | sfdisk $(ISO)



boot: $(BOOT_OBJS)
	ld $^ --oformat elf64-x86-64 -Tboot.ld -o bin/boot.elf
	objcopy --only-keep-debug bin/boot.elf bin/boot_symbols.sym
	objcopy -I elf64-x86-64 -O binary bin/boot.elf bin/boot.bin

kernel: $(KERNEL_OBJS)
	echo $^
	ld.lld $^ --oformat elf64-x86-64 -T kernel.ld  -o bin/kernel.elf
	objcopy --only-keep-debug bin/kernel.elf bin/kernel_symbols.sym
	objcopy -I elf64-x86-64 -O binary bin/kernel.elf bin/kernel.bin

	
clean:
	rm $(BOOT_OBJS) $(KERNEL_OBJS) bin/*
	 
