AS=as
CC=clang

CC_FLAGS+= -c -ffreestanding -target x86_64-none-elf
ifeq ($(MAKECMDGOALS), test)
	CC_FLAGS += -D TEST=1	
endif

BOOT_SRCS= src/boot.s src/stage2.s
BOOT_OBJS= $(BOOT_SRCS:.s=.o)

#KERNEL_SRCS= $(wildcard src/*.c)
KERNEL_SRCS= src/kernel.c src/vga.c src/cpu_ports.c src/unit_tests.c src/mem.c
KERNEL_OBJS=$(KERNEL_SRCS:.c=.o)

ISO=bin/os.bin

all: boot run

%.o: %.s
	$(AS) -o $@ -c $<

%.o: %.c	
	$(CC) $(CC_FLAGS) -o $@ -c $<

test: run

run: iso
	qemu-system-x86_64 --vga std --no-reboot -d int $(ISO)
debug: iso
	qemu-system-x86_64 -s -S --vga std --no-reboot -d int $(ISO)

iso: boot kernel
	dd if=/dev/zero of=$(ISO) bs=512 count=54
	dd if=./bin/boot.bin of=$(ISO) conv=notrunc bs=512 seek=0 count=2 
	dd if=./bin/kernel.bin of=$(ISO) conv=notrunc bs=512 seek=2 count=10  

boot: $(BOOT_OBJS)
	ld $^ --oformat binary -Tlink.ld -o bin/boot.bin
	 
kernel: $(KERNEL_OBJS)
	ld.lld $^ --oformat binary -Ttext=0x8000 -o bin/kernel.bin

	
clean:
	rm src/*.o bin/*
	 
	 
