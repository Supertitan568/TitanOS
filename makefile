AS=as
CC=clang

CC_FLAGS+= -c -ffreestanding -target x86_64-none-elf -fno-integrated-as -mcmodel=large -g
ifeq ($(MAKECMDGOALS), test)
	CC_FLAGS += -D TEST=1	
endif

BOOT_SRCS= src/boot.s src/stage2.s
BOOT_OBJS= $(BOOT_SRCS:.s=.o)

#KERNEL_SRCS= $(wildcard src/*.c)
KERNELC_SRCS= src/kernel.c src/vga.c src/cpu_ports.c src/unit_tests.c src/mem.c src/isr.c src/acpi.c src/apic.c src/keyboard.c src/kernel_heap_manager.c src/vmm.c src/pmm.c src/vmm_hashmap.c

KERNELS_SRCS= src/isr_stubs.s 
KERNEL_OBJS=$(KERNELC_SRCS:.c=.o)
KERNEL_OBJS	+= $(KERNELS_SRCS:.s=.o)
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
	dd if=./bin/kernel.bin of=$(ISO) conv=notrunc bs=512 seek=2 count=30  

boot: $(BOOT_OBJS)
	ld $^ --oformat binary -Tlink.ld -o bin/boot.bin
	 
kernel: $(KERNEL_OBJS)
	echo $^
	ld.lld $^ --oformat elf64-x86-64 -T kernel.ld  -o bin/kernel.elf
	objcopy --only-keep-debug bin/kernel.elf bin/kernel_symbols.sym
	objcopy -I elf64-x86-64 -O binary bin/kernel.elf bin/kernel.bin

	
clean:
	rm src/*.o bin/*
	 
	 
