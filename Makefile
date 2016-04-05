ARCH		:= x86_64
TARGET		:= src/boot/kernel.elf

ISO			:= himawari.iso

CC			:= clang
#  CC			:= gcc
LD 			:= ld

QEMU		:= qemu-system-x86_64
QEMU_FLAGS 	:= -m 16M -S -gdb tcp::10000

SRC			:= ./src
ISO_ROOT	:= ./root
CONFIG		:= $(SRC)/config

all: $(TARGET)

.PHONY: $(TARGET)
$(TARGET): 
	cd src/; $(MAKE) 

$(ISO): $(TARGET)
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(CONFIG)/grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	cp $(TARGET) $(ISO_ROOT)/boot
	grub-mkrescue -o $@ $(ISO_ROOT)

.PHONY: run
run: $(ISO)
	$(QEMU)	$(QEMU_FLAGS) -cdrom $<
	
.PHONY: clean
clean:
	cd src/; $(MAKE) clean
	rm -rf $(ISO)

export CC LD
