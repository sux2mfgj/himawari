TARGET	:= src/kernel/kernel.efi
BOOT_LOADER	:= src/boot/BOOTX64.efi

ARCH	:= x86_64

CC		:= clang
LD		:= ld
CFLAGS	:= -Wall -ggdb3
EFI_CFLAGS	:= -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPER $(CFLAGS)
LDFLAGS	:= -nostdlib

QEMU	:= qemu-system-x86_64

OVMF	:= OVMF.fd

LIB_PATH	:= /usr/lib
EFI_PATH	:= /usr/include/efi
EFI_INCLUDES := -I $(EFI_PATH) -I $(EFI_PATH)/$(ARCH)

HDA		:= run/hda-contents
EFI_BOOT:= $(HDA)/EFI/BOOT/

.PHONY:all
all: $(BOOT_LOADER) $(TARGET)

.PHONY:run
run: run/$(OVMF) $(TARGET) $(EFI_BOOT) $(BOOT_LOADER)
	cp $(BOOT_LOADER) $(EFI_BOOT)
	cp $(TARGET) $(HDA)
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

export CC LD CFLAGS LDFLAGS EFI_CFLAGS LIB_PATH EFI_PATH EFI_INCLUDES ARCH
