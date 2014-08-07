CC = clang
AS = as
LD = ld
QEMU = qemu-system-i386

QEMUFLAGS = -monitor stdio -m 32 -gdb tcp::10000
CFLAGS = -fno-builtin -nostdlib -mno-red-zone -ffreestanding \
			-fno-stack-protector --target=i686-elf -m32
ASFLAGS = --32
LDFLAGS = -m elf_i386 --format elf32-i386 --oformat elf32-i386 -Map kernel.map -Tld.script

C_OBJS = entry.o main.o segment.o interrupt_handler.o graphic.o k_memory.o lib.o task.o
C_SRC = ${C_OBJS: .o=.c}
ASM_OBJS = func.o
OBJS = $(ASM_OBJS) $(C_OBJS)

IMAGE = himawari
ISO = ../himawari.iso

.SUFFIXES: .o .c. s
.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(AS) $(ASFLAGS) -o $@ $<

$(IMAGE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(ISO): $(IMAGE)
	cp $(IMAGE) ../boot
	grub-mkrescue -o $(ISO) ../

run: $(IMAGE)
	$(QEMU) $(QEMUFLAGS) -kernel $(IMAGE)

run_cdrom: $(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(ISO)

clean:
	rm -f $(IMAGE) $(ISO) *.o *.map
