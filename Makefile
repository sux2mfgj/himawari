ARCH	:= x86_64

CC		:= clang
LD		:= ld
OBJCOPY	:= objcopy

CFLAGS	:= -Wall -ggdb3 -fpic -std=c11

QEMU	:= qemu-system-x86_64

OVMF	:= OVMF/OVMF.fd
# OVMF	:= /usr/shared/ovmf/x64/OVMF_CODE.fd

LIB_PATH	:= /usr/lib
EFI_PATH	:= /usr/include/efi
EFI_INCLUDES := -I $(EFI_PATH) -I $(EFI_PATH)/$(ARCH)
EFI_CFLAGS	:= -fno-stack-protector -fshort-wchar -mno-red-zone -DEFI_FUNCTION_WRAPER $(EFI_INCLUDES)
EFI_LDS		:= $(LIB_PATH)/elf_x86_64_efi.lds
CRT0_EFI	:= $(LIB_PATH)/crt0-efi-x86_64.o

LDFLAGS	:= -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic -L $(LIB_PATH) -l:libgnuefi.a -l:libefi.a

HDA		:= fs
EFI_BOOT:= $(HDA)/EFI/BOOT/
BOOT_EFI:= $(EFI_BOOT)/BOOTX64.EFI

QEMUFLAGS	:= -bios $(OVMF) -hda fat:$(HDA) -m 64M
OBJS	:= BOOTX64.o
TMP_SO	:= BOOTX64.so

.PHONY: build
build: $(BOOT_EFI)
$(BOOT_EFI): $(TMP_SO) $(EFI_BOOT)
	$(OBJCOPY) \
		-j .text                \
		-j .sdata               \
		-j .data                \
		-j .dynamic             \
		-j .dynsym              \
		-j .rel                 \
		-j .rela                \
		-j .reloc               \
		--target=efi-app-x86_64 \
		$< $@

.SUFFIXES: .c.o
.c.o:
	$(CC) $(CFLAGS) $(EFI_CFLAGS) -c -o $@ $<

$(TMP_SO): $(OBJS) $(CRT0_EFI)
	$(LD) $^ $(LDFLAGS) -o $@

boot: $(BOOT_EFI) #$(OVMF)
	$(QEMU) $(QEMUFLAGS)

$(EFI_BOOT):
	mkdir -p $@

.PHONY: ovmf
ovmf: $(OVMF)
$(OVMF):
	mkdir -p OVMF
	wget https://sourceforge.net/projects/edk2/files/OVMF/OVMF-X64-r15214.zip -O OVMF/OVMF.zip
	cd OVMF && unzip OVMF.zip OVMF.fd

.PHONY: clean
clean:
	rm -rf fs *.o *.so OVMF
