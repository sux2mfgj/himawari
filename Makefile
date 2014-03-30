
CC = gcc
LD = ld
QEMU = qemu-system-i386
CFLAGS = -fno-builtin -nostdlib -mno-red-zone -ffreestanding -nostdinc -fno-stack-protector

IMAGE = entry.elf


all:
		as entry.s -o entry.o
		$(CC) $(CFLAGS) -c main.c -o main.o
		$(LD) -Tld.script entry.o main.o -o $(IMAGE)

run:
		$(QEMU) -kernel $(IMAGE)



