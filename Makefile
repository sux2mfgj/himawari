ARCH		:= x86_64
KERNEL_PATH	:= src/boot
export KERNEL		:= kernel.elf 

BUILTIN_SERVERS	:= #memory #schedule
#BUILTIN_DEVICE_DRIVER :=
BUILTINS	:= $(foreach dir, $(BUILTIN_SERVERS), src/server/$(dir).elf)

TARGET 	:= $(KERNEL) $(BUILTIN_SERVERS)

ISO			:= himawari.iso

#export RUSTC := $(shell pwd)/rust/bin/rustc
#export CARGO := $(shell pwd)/rust/bin/cargo
#export CC	:= clang
GDB			:= ./gdb
#  CC			:= gcc
export LD 	:= ld

export CFLAGS		:= -Wall -g -ffreestanding -mcmodel=large -m64 \
	-mno-red-zone -mno-mmx -mno-sse -mno-sse2 -std=c11 -nostdlib

QEMU		:= qemu-system-x86_64
QEMU_FLAGS 	:= -m 128M -monitor stdio -gdb tcp::10000 -display none
#QEMU_FLAGS 	:= -m 128M -serial mon:stdio -gdb tcp::10000
QEMU_DEBUG	:= -S

SRC			:= ./src
ISO_ROOT	:= ./root
CONFIG		:= $(SRC)/config

all: $(TARGET) 

.PHONY: $(TARGET)
$(TARGET): #$(RUSTC)
	cd src/; $(MAKE) all

$(ISO): $(KERNEL) #$(BUILTIN_SERVERS)
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(CONFIG)/grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	cp $(KERNEL_PATH)/$(KERNEL) $(ISO_ROOT)/boot
	for file in $(BUILTINS); do\
		cp $$file $(ISO_ROOT)/boot;\
	done
	grub-mkrescue -o $@ $(ISO_ROOT)

#$(RUSTC):
#	curl -f -L https://static.rust-lang.org/rustup.sh -O
#	sh rustup.sh --disable-sudo --disable-ldconfig --prefix=`pwd`/rust --channel=nightly

.PHONY: run
run: $(ISO)
	$(QEMU)	$(QEMU_FLAGS) $(QEMU_DEBUG) -cdrom $<

.PHONY: debug
debug: $(CONFIG)/gdb.conf
	$(GDB) -x $<


	
.PHONY: clean
clean:
	cd src/; $(MAKE) clean
	rm -rf $(ISO)

