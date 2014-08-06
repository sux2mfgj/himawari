
CC = clang
AS = as
LD = ld
QEMU = qemu-system-i386
QEMUFLAGS = -monitor stdio -m 32 -gdb tcp::10000
CFLAGS = -fno-builtin -nostdlib -mno-red-zone -ffreestanding -fno-stack-protector --target=i686-elf -m32
ASFLAGS = --32
LDFLAGS = -m elf_i386 --format elf32-i386 --oformat elf32-i386 -Map kernel.map -Tld.script

OBJS = entry.o main.o segment.o func.o interrupt_handler.o graphic.o k_memory.o lib.o task.o
IMAGE = himawari
ISO = himawari.iso

.SUFFIXES: .o .c. s
.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(AS) $(ASFLAGS) -o $@ $<

$(IMAGE): $(OBJS)
#      as --32 entry.s -o entry.o
#      as --32 func.s -o func.o
#      $(CC) $(CFLAGS) -c main.c -o main.o
#      $(CC) $(CFLAGS) -c graphic.c -o graphic.o
#      $(CC) $(CFLAGS) -c segment.c -o segment.o
#      $(CC) $(CFLAGS) -c k_memory.c -o k_memory.o
#      $(CC) $(CFLAGS) -c interrupt_handler.c -o interrupt_handler.o
#      $(CC) $(CFLAGS) -c lib.c -o lib.o
#      $(CC) $(CFLAGS) -c task.c -o task.o
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(ISO): all
	cp $(IMAGE) ../boot
	grub-mkrescue -o ../himawari.iso ../

run: all
	$(QEMU) $(QEMUFLAGS) -kernel $(IMAGE)

run_cdrom: $(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom ../himawari.iso

clean:
	rm $(IMAGE) *.o *.map
