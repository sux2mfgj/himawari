
CC = clang
LD = ld
QEMU = qemu-system-i386
QEMUFLAGS = -monitor stdio -m 32 -kernel
CFLAGS = -fno-builtin -nostdlib -mno-red-zone -ffreestanding -nostdinc -fno-stack-protector --target=i686-elf -m32
LDFLAGS = -m elf_i386 --format elf32-i386 --oformat elf32-i386
IMAGE = himawari

all:
	as --32 entry.s -o entry.o
	as --32 func.s -o func.o
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) $(CFLAGS) -c graphic.c -o graphic.o
	$(CC) $(CFLAGS) -c segment.c -o segment.o
	$(CC) $(CFLAGS) -c memory.c -o memory.o
	#          $(LD) -Tld.script entry.o segment.o func.o graphic.o  main.o -o $(IMAGE)
	$(LD) $(LDFLAGS) -Map kernel.map -Tld.script  entry.o main.o segment.o func.o graphic.o memory.o -o $(IMAGE)


run: all
	$(QEMU) $(QEMUFLAGS) $(IMAGE)

himawari.iso: all
	cp $(IMAGE) ../boot
	grub-mkrescue -o ../himawari.iso ../

run_cdrom: himawari.iso
	$(QEMU) -monitor stdio -m 32 -cdrom ../himawari.iso

clean:
	rm $(IMAGE) *.o *.map
