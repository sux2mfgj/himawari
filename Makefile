
BOOT	:= src/boot/BOOTX64.efi
KERNEL 	:= src/kernel/kernel.elf
TARGET 	:= $(BOOT) $(KERNEL)

ARCH	:= x86_64

CC		:= clang
LD		:= ld
OBJCOPY	:= objcopy

CFLAGS	:= -Wall -ggdb3 -std=c11
EFI_CFLAGS	:= -fno-stack-protector -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPER $(CFLAGS)

QEMU	:= qemu-system-x86_64

OVMF	:= OVMF.fd

LIB_PATH	:= /usr/lib
EFI_PATH	:= /usr/include/efi
EFI_INCLUDES := -I $(EFI_PATH) -I $(EFI_PATH)/$(ARCH)
EFI_LDS		:= $(LIB_PATH)/elf_x86_64_efi.lds
CRT0_EFI	:= $(LIB_PATH)/crt0-efi-x86_64.o

HDA		:= run/hda-contents
EFI_BOOT:= $(HDA)/EFI/BOOT/

RAM_SIZE	:= 64m
QEMUFLAGS	:= -L ./run -bios $(OVMF) -hda fat:rw:$(HDA) -m $(RAM_SIZE)

.PHONY:all
all: $(TARGET)

.PHONY:run
run: run/$(OVMF) $(TARGET) $(EFI_BOOT) Makefile
	cp $(KERNEL) $(EFI_BOOT)
	cp $(BOOT) $(EFI_BOOT)
	$(QEMU) $(QEMUFLAGS)

debug: run/$(OVMF) $(TARGET) $(EFI_BOOT) Makefile
	cp $(KERNEL) $(EFI_BOOT)
	cp $(BOOT) $(EFI_BOOT)
	$(QEMU) $(QEMUFLAGS) -gdb tcp::10000 -S

.PHONY:$(TARGET)
$(TARGET):
	cd src/boot; $(MAKE)
	cd src/kernel; $(MAKE)

$(EFI_BOOT):
	mkdir -p $(HDA)/EFI/BOOT

run/$(OVMF): $(OVMF)
	mkdir -p run
	cp OVMF.fd $@

$(OVMF):
	wget https://sourceforge.net/projects/edk2/files/OVMF/OVMF-X64-r15214.zip
	unzip OVMF-X64-r15214.zip OVMF.fd

clean:
	rm -rf run *.zip $(OVMF)
	cd src/kernel; $(MAKE) clean
	cd src/boot; $(MAKE) clean

export CC LD CFLAGS EFI_CFLAGS LIB_PATH EFI_PATH EFI_INCLUDES ARCH EFI_LDS CRT0_EFI OBJCOPY
