AS=as

BOOT_SRCS= src/boot.s src/stage2.s

BOOT_OBJS= $(BOOT_SRCS:.s=.o)

ISO=bin/os.bin

all: asm run

%.o: %.s
	$(AS) -o $@ -c $<

run: iso
	qemu-system-i386 --vga std --no-reboot -d int $(ISO)

iso: asm
	dd if=/dev/zero of=$(ISO) bs=512 count=2
	dd if=./bin/boot.bin of=$(ISO) conv=notrunc bs=512 seek=0 count=2 

asm: $(BOOT_OBJS)
	echo  $(BOOT_SRCS)
	ld $^ --oformat binary -Tlink.ld -o bin/boot.bin
	 
	#strip --remove-section=.note.gnu.property boot.o 


clean:
	rm *.o bin/boot.bin
	 
	 
