CC = clang
AS = as
LD = ld
QEMU = qemu-system-i386

QEMUFLAGS = -monitor stdio -m 32M -gdb tcp::10000  -S -sdl
CFLAGS = -ggdb3 -fno-builtin -nostdlib -mno-red-zone -ffreestanding \
			-fno-stack-protector --target=i686-elf -m32
ASFLAGS = --32
PPFLAGS = -E
LDFLAGS = -m elf_i386 --format elf32-i386 --oformat elf32-i386 -Map kernel.map -Tld.script

C_OBJS = entry.o main.o segment.o trap.o keyboard.o graphic.o k_memory.o p_memory.o v_memory.o lib.o task.o string.o timer.o
C_SRC = ${C_OBJS: .o=.c}
ASM_OBJS = func.o vectors.o
OBJS = $(ASM_OBJS) $(C_OBJS)

IMAGE = himawari
ISO = ../himawari.iso

.SUFFIXES: .o .c .S
.c.o:
	$(CC) $(CFLAGS) -c $<
.S.o:
	$(CC) $(PPFLAGS) $< > $(join $<, .tmp)
	$(AS) $(ASFLAGS) -o $@ $(join $<, .tmp)

$(IMAGE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	objdump -D $(IMAGE) > asm_himawari
	ctags -R .


$(ISO): $(IMAGE)
	cp $(IMAGE) ../boot
	grub-mkrescue -o $(ISO) ../

#  run: $(IMAGE)
#      $(QEMU) $(QEMUFLAGS) -kernel $(IMAGE)

run_cdrom: $(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(ISO)

clean:
	rm -f $(IMAGE) $(ISO) *.o *.map *.tmp
