QEMU := qemu-system-i386

QEMUFLAGS := -monitor stdio -m 16M -gdb tcp::10000  -S -sdl

KERNEL := ./kernel/himawari
ISO := ../himawari.iso
DIASM := asm_himawari

.PHONY: all $(KERNEL)
all: $(ISO)

$(ISO): $(KERNEL)
	cp $(KERNEL) ../boot
	grub-mkrescue -o $(ISO) ../

$(KERNEL): $(KERNEL_NEED_FILES)
	$(MAKE) -C ./kernel

run_iso: $(ISO)
	$(QEMU) $(QEMUFLAGS) -cdrom $(ISO)

clean:
	cd kernel && $(MAKE) clean
	rm  $(ISO)

