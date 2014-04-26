
CC = gcc
LD = ld
QEMU = qemu-system-i386
QEMUFLAGS = -monitor stdio -kernel
CFLAGS = -fno-builtin -nostdlib -mno-red-zone -ffreestanding -nostdinc -fno-stack-protector
IMAGE = himawari

all:
		as entry.s -o entry.o
		as func.s -o func.o
		$(CC) $(CFLAGS) -c main.c -o main.o
		$(CC) $(CFLAGS) -c graphic.c -o graphic.o
		$(CC) $(CFLAGS) -c segment.c -o segment.o
#          $(LD) -Tld.script entry.o segment.o func.o graphic.o  main.o -o $(IMAGE)
		$(LD) -Map kernel.map -Tld.script  entry.o main.o segment.o func.o graphic.o -o $(IMAGE)


run:
		$(QEMU) $(QEMUFLAGS) $(IMAGE)


clean:
		rm $(IMAGE) *.o *.map
