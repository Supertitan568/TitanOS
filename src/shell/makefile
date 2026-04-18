CC=clang
CC_FLAGS+= -ffreestanding -target x86_64-none-elf -fno-integrated-as -mcmodel=large -g -nostdlib -mno-avx -mno-sse -Wl,--image-base=0xc0004000 -no-pie

SRCS := $(shell find ./ -name '*.c')
OBJS := $(SRCS:.c=.c.o)

OUTPUT=shell.elf

all: link

link: $(OBJS)
	$(CC) $(OBJS) $(CC_FLAGS) -o $(OUTPUT)
%.c.o: %.c	
	$(CC) $(CC_FLAGS) -o $@ -c $<

