QEMU := qemu-system-i386

QEMUFLAGS := -monitor stdio -m 16M -sdl
QEMU_DEBUG_FLAGS = -S -gdb tcp::10000

KERNEL := ./kernel/himawari
ISO := ../himawari.iso
DIASM := asm_himawari
EFI_IMG := ../efi.img

.PHONY: all $(KERNEL)
all: $(ISO)

$(ISO): $(KERNEL)
	cp $(KERNEL) ../boot
	grub-mkrescue -o $(ISO) ../

$(KERNEL): $(KERNEL_NEED_FILES)
	$(MAKE) -C ./kernel

run_debug: $(ISO)
	$(QEMU) $(QEMUFLAGS) $(QEMU_DEBUG_FLAGS) -cdrom $(ISO)

$(EFI_IMG):
	grub-mkimage -O i386-pc -o ../efi.img multiboot

run_iso: $(ISO) $(EFI_IMG)
	$(QEMU) $(QEMUFLAGS) -cdrom $(ISO)

clean:
	cd kernel && $(MAKE) clean
	rm  $(ISO)

