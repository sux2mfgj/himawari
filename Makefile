TARGET	:= src/kernel/BOOTX64.efi
#  BOOT_LOADER	:= src/boot/BOOTX64.efi

ARCH	:= x86_64

CC		:= clang
LD		:= ld
OBJCOPY	:= objcopy

CFLAGS	:= -Wall -ggdb3
EFI_CFLAGS	:= -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPER $(CFLAGS)
#  LDFLAGS	:= -nostdlib


QEMU	:= qemu-system-x86_64

OVMF	:= OVMF.fd

LIB_PATH	:= /usr/lib
EFI_PATH	:= /usr/include/efi
EFI_INCLUDES := -I $(EFI_PATH) -I $(EFI_PATH)/$(ARCH)
EFI_LDS		:= $(LIB_PATH)/elf_x86_64_efi.lds
CRT0_EFI	:= $(LIB_PATH)/crt0-efi-x86_64.o

HDA		:= run/hda-contents
EFI_BOOT:= $(HDA)/EFI/BOOT/

QEMUFLAGS	:= -L ./run -bios $(OVMF) -hda fat:$(HDA) -m 64M

.PHONY:all
all: $(TARGET)

.PHONY:run
run: run/$(OVMF) $(TARGET) $(EFI_BOOT) Makefile
	cp $(TARGET) $(EFI_BOOT)
#      cp $(TARGET) $(HDA)
#      $(QEMU) -L ./run -bios $(OVMF) -hda fat:$(HDA)
	$(QEMU) $(QEMUFLAGS)

#  .PHONY:$(BOOT_LOADER)
#  $(BOOT_LOADER):
#      cd src/boot; $(MAKE)

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

export CC LD CFLAGS EFI_CFLAGS LIB_PATH EFI_PATH EFI_INCLUDES ARCH EFI_LDS CRT0_EFI OBJCOPY
