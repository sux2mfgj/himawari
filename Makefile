ARCH		:= x86_64
TARGET		:= src/boot/kernel.elf

ISO			:= himawari.iso

export CC			:= clang
#  CC			:= gcc
export LD 			:= ld

export CFLAGS		:= -Wall -g -ffreestanding -mcmodel=large -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2

QEMU		:= qemu-system-x86_64
QEMU_FLAGS 	:= -m 128M  -monitor stdio -gdb tcp::10000
QEMU_DEBUG	:= -S 


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
	$(QEMU)	$(QEMU_FLAGS) $(QEMU_DEBUG) -cdrom $<

.PHONY: debug
debug: $(CONFIG)/gdb.conf
	gdb -x $<
	
.PHONY: clean
clean:
	cd src/; $(MAKE) clean
	rm -rf $(ISO)

