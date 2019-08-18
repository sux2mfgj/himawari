
BOOT	:= src/boot/BOOTX64.efi
KERNEL 	:= src/kernel/kernel.elf
TARGET 	:= $(BOOT) $(KERNEL)

ARCH	:= x86_64

CC		:= clang
LD		:= ld
OBJCOPY	:= objcopy

CFLAGS	:= -Wall -ggdb3 -std=c11 -fno-stack-protector
EFI_CFLAGS	:= -fno-stack-protector -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPER $(CFLAGS)

QEMU	:= qemu-system-x86_64

OVMF=/usr/share/ovmf/x64/OVMF_CODE.fd

LIB_PATH	:= /usr/lib
EFI_PATH	:= /usr/include/efi
EFI_INCLUDES := -I $(EFI_PATH) -I $(EFI_PATH)/$(ARCH)
EFI_LDS		:= $(LIB_PATH)/elf_x86_64_efi.lds
CRT0_EFI	:= $(LIB_PATH)/crt0-efi-x86_64.o

HDA		:= run/hda-contents
EFI_BOOT:= $(HDA)/EFI/BOOT/

RAM_SIZE	:= 1G
QEMU_DEVS	:= -monitor stdio -machine q35 \
-device virtio-serial-pci,bus=pcie.0 \
-nodefaults -vga std
#-device pcie-root-port,id=root,slot=0
#-device pcie-root-port,id=root,slot=1
#	-device virtio-serial-pci,id=virtio-serial0 -device virtconsole,chardev=charconsole0,id=console0 -chardev stdio,id=charconsole0 \
#	-device pcie-root-port,id=rp20,bus=pcie.0,chassis=1,addr=2.0,multifunction=on,pref64-reserve=32M

#-device virtio-net-pci,netdev=net0 -netdev socket,id=net0,listen=:5002

QEMUFLAGS	:= -L ./run -bios $(OVMF) -hda fat:rw:$(HDA) -m $(RAM_SIZE) $(QEMU_DEVS)

.PHONY:all
all: $(TARGET)

.PHONY:run
run: $(OVMF) $(TARGET) $(EFI_BOOT) Makefile
	cp $(KERNEL) $(EFI_BOOT)
	cp $(BOOT) $(EFI_BOOT)
	$(QEMU) $(QEMUFLAGS)

debug: $(OVMF) $(TARGET) $(EFI_BOOT) Makefile
	cp $(KERNEL) $(EFI_BOOT)
	cp $(BOOT) $(EFI_BOOT)
	$(QEMU) $(QEMUFLAGS) -gdb tcp::10000 -S

.PHONY:$(TARGET)
$(TARGET):
	cd src/boot; $(MAKE)
	cd src/kernel; $(MAKE)

$(EFI_BOOT):
	mkdir -p $(HDA)/EFI/BOOT

clean:
	rm -rf run *.zip $(OVMF)
	cd src/kernel; $(MAKE) clean
	cd src/boot; $(MAKE) clean

export CC LD CFLAGS EFI_CFLAGS LIB_PATH EFI_PATH EFI_INCLUDES ARCH EFI_LDS CRT0_EFI OBJCOPY
