
CC = gcc
LD = ld
QEMU = qemu-system-i386
CFLAGS = -fno-builtin -nostdlib -mno-red-zone -ffreestanding -nostdinc -fno-stack-protector
IMAGE = entry.elf

all:
		as entry.s -o entry.o
		as func.s -o func.o
		$(CC) $(CFLAGS) -c main.c -o main.o
		$(CC) $(CFLAGS) -c graphic.c -o graphic.o
		$(CC) $(CFLAGS) -c segment.c -o segment.o
		$(LD) -Tld.script entry.o func.o graphic.o segment.o main.o -o $(IMAGE)

run:
		$(QEMU) -kernel $(IMAGE)


clean:
		rm $(IMAGE) *.o
