TARGET	:= src/kernel/kernel.elf
BOOT_LOADER	:= src/boot/BOOTX64.efi

QEMU	:= qemu-system-x86_64

OVMF	:= OVMF.fd

HDA		:= run/hda-contents
EFI_BOOT:= $(HDA)/EFI/BOOT/

.PHONY:all
all: $(BOOT_LOADER) $(TARGET)

.PHONY:run
run: run/$(OVMF) $(TARGET) $(EFI_BOOT) $(BOOT_LOADER)
	cp $(BOOT_LOADER) $(EFI_BOOT)
	$(QEMU) -L ./run -bios $(OVMF) -hda fat:$(HDA)

.PHONY:$(BOOT_LOADER)
$(BOOT_LOADER):
	cd src/boot; $(MAKE)

.PHONY:$(TARGET)
$(TARGET):
	cd src/kernel; $(MAKE)

$(EFI_BOOT):
	mkdir -p $(HDA)/EFI/BOOT

run/$(OVMF):
	mkdir -p run
	wget https://sourceforge.net/projects/edk2/files/OVMF/OVMF-X64-r15214.zip 
	unzip OVMF-X64-r15214.zip OVMF.fd
	mv OVMF.fd $@

clean:
	cd src/boot; $(MAKE) clean
	cd src/kernel; $(MAKE) clean
